/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TESTBENCH_H
#define TESTBENCH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <libpldm/base.h>

#define TT_PLDM_OEM_TYPE                 0x3FU
#define TT_PLDM_OEM_CMD_SHELL_EXEC       0x01U
#define TT_PLDM_OEM_CMD_SHELL_GET_RESULT 0x02U
#define TT_PLDM_OEM_CMD_SHELL_CANCEL     0x03U

bool dmc_mctp_transport_ready(void);
uint32_t ConvertFloatToTelemetry(float value);
void UpdateTelemetryTag(uint16_t tag, uint32_t value);

/* Transport globals accessible to tests */
extern uint8_t g_rx_buf[96];
extern size_t g_rx_len;
extern const struct pldm_msg *g_resp;
extern size_t g_resp_pldm_payload_len;

/* Transport helpers */
void dmc_mctp_transport_rx_reset(void);
uint32_t dmc_mctp_transport_rx_count_get(void);
size_t dmc_mctp_transport_last_rx_len_get(void);
size_t dmc_mctp_transport_last_rx_copy(uint8_t *buf, size_t buf_size);

int dmc_mctp_transport_send_pldm_request(const void *pldm_req, size_t pldm_req_len);

/* Response waiting and validation */
void wait_for_pldm_response_view(void);
void validate_response_header(const struct pldm_msg *req, const struct pldm_msg *resp,
			      uint8_t expected_cmd, uint8_t expected_pldm_type);
void validate_cc_only_response(const struct pldm_msg *resp, size_t resp_pldm_payload_len,
			       uint8_t expected_cc);

/* Request senders */
int send_get_tid_request(struct pldm_msg *req);
int send_set_tid_request(struct pldm_msg *req, uint8_t instance_id, uint8_t tid);
int send_get_types_request(struct pldm_msg *req);
int send_get_version_request(struct pldm_msg *req);
int send_get_version_request_for_type(struct pldm_msg *req, uint8_t instance_id,
				      uint8_t transfer_opflag, uint8_t pldm_type);
int send_get_commands_request_for_type(struct pldm_msg *req, uint8_t instance_id,
				       uint8_t pldm_type);
int send_get_commands_request(struct pldm_msg *req);
int send_get_pdr_repository_info_request(struct pldm_msg *req);
int send_get_pdr_request(struct pldm_msg *req, uint8_t instance_id, uint32_t record_handle,
			 uint32_t data_transfer_handle, uint8_t transfer_op_flag,
			 uint16_t request_count, uint16_t record_change_number);
int send_get_sensor_reading_request(struct pldm_msg *req, uint8_t instance_id, uint16_t sensor_id,
				    bool8_t rearm_event_state);
int send_oem_shell_exec_request(struct pldm_msg *req, uint8_t instance_id, uint16_t request_id,
				const char *cmd);
int send_oem_shell_get_result_request(struct pldm_msg *req, uint8_t instance_id,
				      uint16_t request_id, uint16_t offset, uint8_t max_read_len);
void send_custom_pldm_request(uint8_t instance, uint8_t pldm_type, uint8_t cmd, const void *payload,
			      size_t payload_len);

/* Controller initialization */
int dmc_mctp_transport_test_controller_init(void);

#endif /* TESTBENCH_H */
