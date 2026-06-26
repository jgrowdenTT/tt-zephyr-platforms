/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <libmctp.h>
#include <libpldm/base.h>
#include <libpldm/platform.h>
#include <zephyr/drivers/smbus.h>
#include <zephyr/pmci/mctp/mctp_i3c_controller.h>

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zephyr/sys/printk.h>
#include <zephyr/ztest.h>

#include "testbench.h"

bool dmc_mctp_transport_ready(void);

MCTP_I3C_CONTROLLER_DT_DEFINE(dmc_mctp_i3c_ctrl, DT_NODELABEL(mctp_i3c));

#define DMC_MCTP_LOCAL_EID     20U
#define DMC_MCTP_TARGET_EID    11U
#define DMC_MCTP_MSG_TYPE_PLDM 0x01U

#define pldm_error(fmt, ...) printk("PLDM ERROR: " fmt "\n", ##__VA_ARGS__)

/* Transport globals */
static struct mctp *mctp_controller_ctx;

static volatile uint32_t dmc_mctp_rx_count;
static volatile size_t dmc_mctp_last_rx_len;
static uint8_t dmc_mctp_last_rx_data[96];
static volatile size_t dmc_mctp_last_rx_data_len;

/* Shared response scratch space used by all tests in this suite. */
uint8_t g_rx_buf[96];
size_t g_rx_len;
const struct pldm_msg *g_resp;
size_t g_resp_pldm_payload_len;

static void dmc_mctp_transport_test_record_rx(const void *msg, size_t len);

static int dmc_device_init_if_needed(const struct device *dev, const char *name)
{
	int rc;

	rc = device_init(dev);
	if (rc == 0 || rc == -EALREADY) {
		return 0;
	}

	ARG_UNUSED(name);
	return rc;
}

static void dmc_mctp_controller_rx_message(uint8_t eid, bool tag_owner, uint8_t msg_tag, void *data,
					   void *msg, size_t len)
{
	ARG_UNUSED(eid);
	ARG_UNUSED(tag_owner);
	ARG_UNUSED(msg_tag);
	ARG_UNUSED(data);

	dmc_mctp_transport_test_record_rx(msg, len);
}

int dmc_mctp_transport_test_controller_init(void)
{
	int rc;
	const struct device *i3c_controller = DEVICE_DT_GET(DT_NODELABEL(i3c2));
	const struct device *mctp_endpoint = DEVICE_DT_GET(DT_NODELABEL(mctp_endpoint0));
	const struct device *smbus = DEVICE_DT_GET(DT_NODELABEL(smbus0));

	if (mctp_controller_ctx != NULL) {
		return 0;
	}

	rc = dmc_device_init_if_needed(i3c_controller, "i3c controller");
	if (rc != 0) {
		for (int attempt = 1; rc == -EIO && attempt <= 5; attempt++) {
			k_msleep(10);
			rc = dmc_device_init_if_needed(i3c_controller, "i3c controller");
		}

		if (rc != 0) {
			return rc;
		}
	}

	rc = dmc_device_init_if_needed(mctp_endpoint, "mctp endpoint");
	if (rc != 0) {
		return rc;
	}

	rc = dmc_device_init_if_needed(smbus, "smbus");
	if (rc != 0) {
		return rc;
	}

	rc = smbus_configure(smbus, SMBUS_MODE_CONTROLLER | SMBUS_MODE_PEC);
	if (rc != 0) {
		return rc;
	}

	mctp_controller_ctx = mctp_init();
	if (mctp_controller_ctx == NULL) {
		return -ENOMEM;
	}

	rc = mctp_register_bus(mctp_controller_ctx, &dmc_mctp_i3c_ctrl.binding, DMC_MCTP_LOCAL_EID);
	if (rc != 0) {
		return rc;
	}

	mctp_set_rx_all(mctp_controller_ctx, dmc_mctp_controller_rx_message, NULL);

	return 0;
}

static int dmc_mctp_transport_test_send_raw(const void *buf, size_t len, bool tag_owner,
					    uint8_t msg_tag)
{
	if (!dmc_mctp_transport_ready() || mctp_controller_ctx == NULL) {
		return -ENODEV;
	}

	if (buf == NULL || len == 0U) {
		return -EINVAL;
	}

	return mctp_message_tx(mctp_controller_ctx, DMC_MCTP_TARGET_EID, tag_owner, msg_tag,
			       (void *)buf, len);
}

static void dmc_mctp_transport_test_record_rx(const void *msg, size_t len)
{
	unsigned int key;
	size_t copy_len = MIN(len, sizeof(dmc_mctp_last_rx_data));

	key = irq_lock();
	dmc_mctp_rx_count++;
	dmc_mctp_last_rx_len = len;
	dmc_mctp_last_rx_data_len = copy_len;
	memcpy(dmc_mctp_last_rx_data, msg, copy_len);
	irq_unlock(key);
}

void dmc_mctp_transport_rx_reset(void)
{
	unsigned int key;

	key = irq_lock();
	dmc_mctp_rx_count = 0U;
	dmc_mctp_last_rx_len = 0U;
	dmc_mctp_last_rx_data_len = 0U;
	irq_unlock(key);
}

uint32_t dmc_mctp_transport_rx_count_get(void)
{
	return dmc_mctp_rx_count;
}

size_t dmc_mctp_transport_last_rx_len_get(void)
{
	return dmc_mctp_last_rx_len;
}

size_t dmc_mctp_transport_last_rx_copy(uint8_t *buf, size_t buf_size)
{
	size_t copy_len = MIN(dmc_mctp_last_rx_data_len, buf_size);

	if (copy_len > 0U) {
		memcpy(buf, dmc_mctp_last_rx_data, copy_len);
	}

	return copy_len;
}

int dmc_mctp_transport_send_pldm_request(const void *pldm_req, size_t pldm_req_len)
{
	uint8_t tx_buf[1U + 64U];

	if (pldm_req == NULL || pldm_req_len == 0U) {
		return -EINVAL;
	}

	if (pldm_req_len > sizeof(tx_buf) - 1U) {
		return -EMSGSIZE;
	}

	tx_buf[0] = DMC_MCTP_MSG_TYPE_PLDM;
	memcpy(&tx_buf[1], pldm_req, pldm_req_len);

	return dmc_mctp_transport_test_send_raw(tx_buf, 1U + pldm_req_len, true, 0);
}

static void wait_for_pldm_response(uint8_t *rx_buf, size_t rx_buf_size, size_t *rx_len_out)
{
	int64_t deadline = k_uptime_get() + 500;

	do {
		if (dmc_mctp_transport_rx_count_get() > 0U) {
			unsigned int key;
			size_t rx_len;
			size_t rx_copy_len;

			key = irq_lock();
			rx_len = dmc_mctp_last_rx_len;
			rx_copy_len = MIN(dmc_mctp_last_rx_data_len, rx_buf_size);
			if (rx_copy_len > 0U) {
				memcpy(rx_buf, dmc_mctp_last_rx_data, rx_copy_len);
			}
			irq_unlock(key);

			zassert_true(rx_len >= 1U, "short RX len=%u", (unsigned int)rx_len);
			zassert_equal(rx_copy_len, rx_len, "RX truncation copied=%u total=%u",
				      (unsigned int)rx_copy_len, (unsigned int)rx_len);
			zassert_equal(rx_buf[0], DMC_MCTP_MSG_TYPE_PLDM,
				      "wrong MCTP msg type 0x%02x", rx_buf[0]);

			*rx_len_out = rx_len;
			return;
		}

		k_msleep(10);
	} while (k_uptime_get() < deadline);

	zassert_true(false, "timeout waiting for PLDM response");
}

void wait_for_pldm_response_view(void)
{
	wait_for_pldm_response(g_rx_buf, sizeof(g_rx_buf), &g_rx_len);
	g_resp = (const struct pldm_msg *)&g_rx_buf[1];
	g_resp_pldm_payload_len = (g_rx_len - 1U) - sizeof(struct pldm_msg_hdr);
}

void validate_response_header(const struct pldm_msg *req, const struct pldm_msg *resp,
			      uint8_t expected_cmd, uint8_t expected_pldm_type)
{
	struct pldm_header_info req_hdr;
	struct pldm_header_info resp_hdr;
	uint8_t cc;

	cc = unpack_pldm_header(&req->hdr, &req_hdr);
	zassert_equal(cc, PLDM_SUCCESS, "unpack req hdr rc=%u", cc);

	cc = unpack_pldm_header(&resp->hdr, &resp_hdr);
	zassert_equal(cc, PLDM_SUCCESS, "unpack resp hdr rc=%u", cc);

	zassert_true(resp_hdr.msg_type == PLDM_RESPONSE && resp_hdr.command == expected_cmd &&
			     resp_hdr.instance == req_hdr.instance &&
			     resp_hdr.pldm_type == expected_pldm_type,
		     "bad resp hdr type=%u cmd=%u inst=%u pldm_type=%u", resp_hdr.msg_type,
		     resp_hdr.command, resp_hdr.instance, resp_hdr.pldm_type);
}

void send_custom_pldm_request(uint8_t instance, uint8_t pldm_type, uint8_t cmd, const void *payload,
			      size_t payload_len)
{
	uint8_t req_buf[sizeof(struct pldm_msg_hdr) + 16U];
	struct pldm_msg *req = (struct pldm_msg *)req_buf;
	struct pldm_header_info hdr = {
		.msg_type = PLDM_REQUEST,
		.instance = instance,
		.pldm_type = pldm_type,
		.command = cmd,
	};
	uint8_t cc;
	int rc;

	zassert_true(payload_len <= sizeof(req_buf) - sizeof(struct pldm_msg_hdr),
		     "custom req payload too large len=%u max=%u", (unsigned int)payload_len,
		     (unsigned int)(sizeof(req_buf) - sizeof(struct pldm_msg_hdr)));

	cc = pack_pldm_header(&hdr, &req->hdr);
	zassert_equal(cc, PLDM_SUCCESS, "pack custom req hdr rc=%u", cc);

	if (payload_len > 0U && payload != NULL) {
		memcpy(req_buf + sizeof(struct pldm_msg_hdr), payload, payload_len);
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr) + payload_len);
	zassert_true(rc >= 0, "send custom req rc=%d", rc);
}

void validate_cc_only_response(const struct pldm_msg *resp, size_t resp_pldm_payload_len,
			       uint8_t expected_cc)
{
	const uint8_t *payload = (const uint8_t *)resp + sizeof(struct pldm_msg_hdr);

	zassert_equal(resp_pldm_payload_len, 1U, "expected cc-only payload len=1 got=%u",
		      (unsigned int)resp_pldm_payload_len);
	zassert_equal(payload[0], expected_cc, "wrong completion code got=%u expected=%u",
		      payload[0], expected_cc);
}

int send_get_tid_request(struct pldm_msg *req)
{
	int rc;

	rc = encode_get_tid_req(1U, req);
	if (rc != PLDM_SUCCESS) {
		pldm_error("FAIL: encode GetTID rc=%d", rc);
		return -EINVAL;
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr));
	if (rc < 0) {
		pldm_error("FAIL: send GetTID rc=%d", rc);
	}

	return rc;
}

int send_set_tid_request(struct pldm_msg *req, uint8_t instance_id, uint8_t tid)
{
	int rc;

	rc = encode_set_tid_req(instance_id, tid, req);
	if (rc != PLDM_SUCCESS) {
		pldm_error("FAIL: encode SetTID tid=%u rc=%d", tid, rc);
		return -EINVAL;
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr) +
							       PLDM_SET_TID_REQ_BYTES);
	if (rc < 0) {
		pldm_error("FAIL: send SetTID tid=%u rc=%d", tid, rc);
	}

	return rc;
}

int send_get_types_request(struct pldm_msg *req)
{
	int rc;

	rc = encode_get_types_req(2U, req);
	if (rc != PLDM_SUCCESS) {
		pldm_error("FAIL: encode GetPLDMTypes rc=%d", rc);
		return -EINVAL;
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr));
	if (rc < 0) {
		pldm_error("FAIL: send GetPLDMTypes rc=%d", rc);
	}

	return rc;
}

int send_get_version_request(struct pldm_msg *req)
{
	int rc;

	rc = encode_get_version_req(3U, 0U, PLDM_GET_FIRSTPART, PLDM_BASE, req);
	if (rc != PLDM_SUCCESS) {
		pldm_error("FAIL: encode GetPLDMVersion rc=%d", rc);
		return -EINVAL;
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr) +
							       PLDM_GET_VERSION_REQ_BYTES);
	if (rc < 0) {
		pldm_error("FAIL: send GetPLDMVersion rc=%d", rc);
	}

	return rc;
}

int send_get_version_request_for_type(struct pldm_msg *req, uint8_t instance_id,
				      uint8_t transfer_opflag, uint8_t pldm_type)
{
	int rc;

	rc = encode_get_version_req(instance_id, 0U, transfer_opflag, pldm_type, req);
	if (rc != PLDM_SUCCESS) {
		pldm_error("FAIL: encode GetPLDMVersion type=%u opflag=%u rc=%d", pldm_type,
			   transfer_opflag, rc);
		return -EINVAL;
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr) +
							       PLDM_GET_VERSION_REQ_BYTES);
	if (rc < 0) {
		pldm_error("FAIL: send GetPLDMVersion rc=%d", rc);
	}

	return rc;
}

int send_get_commands_request_for_type(struct pldm_msg *req, uint8_t instance_id, uint8_t pldm_type)
{
	int rc;

	rc = encode_get_commands_req(instance_id, pldm_type, (ver32_t){0}, req);
	if (rc != PLDM_SUCCESS) {
		pldm_error("FAIL: encode GetPLDMCommands type=%u rc=%d", pldm_type, rc);
		return -EINVAL;
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr) +
							       PLDM_GET_COMMANDS_REQ_BYTES);
	if (rc < 0) {
		pldm_error("FAIL: send GetPLDMCommands rc=%d", rc);
	}

	return rc;
}

int send_get_commands_request(struct pldm_msg *req)
{
	return send_get_commands_request_for_type(req, 4U, PLDM_BASE);
}

int send_get_pdr_repository_info_request(struct pldm_msg *req)
{
	int rc;

	rc = encode_get_pdr_repository_info_req(8U, req, 0U);
	if (rc != PLDM_SUCCESS) {
		pldm_error("FAIL: encode GetPDRRepositoryInfo rc=%d", rc);
		return -EINVAL;
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr));
	if (rc < 0) {
		pldm_error("FAIL: send GetPDRRepositoryInfo rc=%d", rc);
	}

	return rc;
}

int send_get_pdr_request(struct pldm_msg *req, uint8_t instance_id, uint32_t record_handle,
			 uint32_t data_transfer_handle, uint8_t transfer_op_flag,
			 uint16_t request_count, uint16_t record_change_number)
{
	int rc;

	rc = encode_get_pdr_req(instance_id, record_handle, data_transfer_handle, transfer_op_flag,
				request_count, record_change_number, req, PLDM_GET_PDR_REQ_BYTES);
	if (rc != PLDM_SUCCESS) {
		pldm_error("FAIL: encode GetPDR rc=%d", rc);
		return -EINVAL;
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr) +
							       PLDM_GET_PDR_REQ_BYTES);
	if (rc < 0) {
		pldm_error("FAIL: send GetPDR rc=%d", rc);
	}

	return rc;
}

int send_get_sensor_reading_request(struct pldm_msg *req, uint8_t instance_id, uint16_t sensor_id,
				    bool8_t rearm_event_state)
{
	int rc;

	rc = encode_get_sensor_reading_req(instance_id, sensor_id, rearm_event_state, req);
	if (rc != PLDM_SUCCESS) {
		pldm_error("FAIL: encode GetSensorReading sensor=%u rc=%d", sensor_id, rc);
		return -EINVAL;
	}

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr) +
							       PLDM_GET_SENSOR_READING_REQ_BYTES);
	if (rc < 0) {
		pldm_error("FAIL: send GetSensorReading sensor=%u rc=%d", sensor_id, rc);
	}

	return rc;
}

int send_oem_shell_exec_request(struct pldm_msg *req, uint8_t instance_id, uint16_t request_id,
				const char *cmd)
{
	struct pldm_header_info hdr = {
		.msg_type = PLDM_REQUEST,
		.instance = instance_id,
		.pldm_type = TT_PLDM_OEM_TYPE,
		.command = TT_PLDM_OEM_CMD_SHELL_EXEC,
	};
	uint8_t cc;
	size_t cmd_len;
	size_t req_pldm_len;
	int rc;

	if (cmd == NULL) {
		return -EINVAL;
	}

	cmd_len = strlen(cmd);
	if (cmd_len == 0U || cmd_len > UINT8_MAX) {
		return -EMSGSIZE;
	}

	cc = pack_pldm_header(&hdr, &req->hdr);
	if (cc != PLDM_SUCCESS) {
		return -EINVAL;
	}

	sys_put_le16(request_id, &req->payload[0]);
	req->payload[2] = (uint8_t)cmd_len;
	memcpy(&req->payload[3], cmd, cmd_len);

	req_pldm_len = sizeof(struct pldm_msg_hdr) + 3U + cmd_len;
	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, req_pldm_len);
	if (rc < 0) {
		pldm_error("FAIL: send OEM shell exec rc=%d", rc);
	}

	return rc;
}

int send_oem_shell_get_result_request(struct pldm_msg *req, uint8_t instance_id,
				      uint16_t request_id, uint16_t offset, uint8_t max_read_len)
{
	struct pldm_header_info hdr = {
		.msg_type = PLDM_REQUEST,
		.instance = instance_id,
		.pldm_type = TT_PLDM_OEM_TYPE,
		.command = TT_PLDM_OEM_CMD_SHELL_GET_RESULT,
	};
	uint8_t cc;
	int rc;

	cc = pack_pldm_header(&hdr, &req->hdr);
	if (cc != PLDM_SUCCESS) {
		return -EINVAL;
	}

	sys_put_le16(request_id, &req->payload[0]);
	sys_put_le16(offset, &req->payload[2]);
	req->payload[4] = max_read_len;

	dmc_mctp_transport_rx_reset();
	rc = dmc_mctp_transport_send_pldm_request(req, sizeof(struct pldm_msg_hdr) + 5U);
	if (rc < 0) {
		pldm_error("FAIL: send OEM shell get-result rc=%d", rc);
	}

	return rc;
}
