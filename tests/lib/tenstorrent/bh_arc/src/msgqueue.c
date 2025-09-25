/*
 * Copyright (c) 2024 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>

#include <tenstorrent/msg_type.h>
#include <tenstorrent/msgqueue.h>
#include "asic_state.h"

#include "reg_mock.h"

/* Custom fake for ReadReg to simulate timer progression */
#define RESET_UNIT_REFCLK_CNT_LO_REG_ADDR 0x800300E0
static uint32_t timer_counter;

static uint32_t ReadReg_timer_fake(uint32_t addr)
{
	/* IC_STATUS; Fake out TX_FIFO to say empty and not full Fake out RX_FIFO to say not empty.
	 * This should be replaced by a emulated i2c driver once we use
	 * a real zephyr i2c controller in our app.
	 */
	if (addr == 0x80090070) {
		return 0b1110;
	}

	if (addr == RESET_UNIT_REFCLK_CNT_LO_REG_ADDR) {
		return timer_counter++;
	}
	return 0;
}

static uint8_t msgqueue_handler_73(const union request *req, struct response *rsp)
{
	BUILD_ASSERT(MSG_TYPE_SHIFT % 8 == 0);
	rsp->data[1] = req->data[0];
	return 0;
}

ZTEST(msgqueue, test_msgqueue_register_handler)
{
	union request req = {0};
	struct response rsp = {0};

	msgqueue_register_handler(0x73, msgqueue_handler_73);

	req.data[0] = 0x73737373;
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[1], 0x73737373);
}

ZTEST(msgqueue, test_msg_type_set_voltage)
{
	union request req = {0};
	struct response rsp = {0};

	timer_counter = 0;
	ReadReg_fake.custom_fake = ReadReg_timer_fake;

	req.data[0] = MSG_TYPE_SET_VOLTAGE;
	req.data[1] = 0x64; /* regulator id */
	req.data[2] = 800;  /* voltage in mV */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_get_voltage)
{
	union request req = {0};
	struct response rsp = {0};

	timer_counter = 0;
	ReadReg_fake.custom_fake = ReadReg_timer_fake;

	req.data[0] = MSG_TYPE_GET_VOLTAGE;
	req.data[1] = 0x64; /* regulator id */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_switch_vout_control)
{
	union request req = {0};
	struct response rsp = {0};

	/* Reset timer counter and set up the fake */
	timer_counter = 0;
	ReadReg_fake.custom_fake = ReadReg_timer_fake;

	req.data[0] = MSG_TYPE_SWITCH_VOUT_CONTROL;
	req.data[1] = 0x01; /* regulator id */
	req.data[2] = 1;    /* enable */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_switch_clk_scheme)
{
	union request req = {0};
	struct response rsp = {0};

	/* Reset timer counter and set up the fake */
	timer_counter = 0;
	ReadReg_fake.custom_fake = ReadReg_timer_fake;

	req.data[0] = MSG_TYPE_SWITCH_CLK_SCHEME;
	req.data[1] = 0x01; /* clock scheme */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_force_aiclk)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_FORCE_AICLK;
	req.data[1] = 1000; /* frequency in MHz */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_get_aiclk)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_GET_AICLK;
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_aiclk_go_busy)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_AICLK_GO_BUSY;
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_aiclk_go_long_idle)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_AICLK_GO_LONG_IDLE;
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_aisweep_start)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_AISWEEP_START;
	req.data[1] = 500;  /* start frequency */
	req.data[2] = 1000; /* end frequency */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_aisweep_stop)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_AISWEEP_STOP;
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_force_vdd)
{
	union request req = {0};
	struct response rsp = {0};

	/* Reset timer counter and set up the fake */
	timer_counter = 0;
	ReadReg_fake.custom_fake = ReadReg_timer_fake;

	req.data[0] = MSG_TYPE_FORCE_VDD;
	req.data[1] = 800; /* voltage in mV */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_force_fan_speed)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_FORCE_FAN_SPEED;
	req.data[1] = 50; /* fan speed percentage */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

#ifdef CONFIG_DT_HAS_TENSTORRENT_BH_PVT_ENABLED
ZTEST(msgqueue, test_msg_type_read_ts)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_READ_TS;
	req.data[1] = 0x01; /* temperature sensor id */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_read_pd)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_READ_PD;
	req.data[1] = 0x01; /* phase detector id */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_read_vm)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_READ_VM;
	req.data[1] = 0x01; /* voltage monitor id */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}
#endif

ZTEST(msgqueue, test_msg_type_send_pcie_msi)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_SEND_PCIE_MSI;
	req.data[1] = 0x01; /* MSI number */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_pcie_dma_host_to_chip_transfer)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_PCIE_DMA_HOST_TO_CHIP_TRANSFER;
	req.data[1] = 0x1000; /* source address */
	req.data[2] = 0x2000; /* destination address */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_pcie_dma_chip_to_host_transfer)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_PCIE_DMA_CHIP_TO_HOST_TRANSFER;
	req.data[1] = 0x3000; /* source address */
	req.data[2] = 0x4000; /* destination address */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_read_eeprom)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_READ_EEPROM;
	req.data[1] = 0x100; /* address */
	req.data[2] = 4;     /* length */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_write_eeprom)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_WRITE_EEPROM;
	req.data[1] = 0x100;      /* address */
	req.data[2] = 0x12345678; /* data */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_trigger_reset)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_TRIGGER_RESET;
	req.data[1] = 3; /* reset type */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_toggle_tensix_reset)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_TOGGLE_TENSIX_RESET;
	req.data[1] = 0x01; /* tensix core mask */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_reinit_tensix)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_REINIT_TENSIX;
	req.data[1] = 0x01; /* tensix core mask */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_debug_noc_translation)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_DEBUG_NOC_TRANSLATION;
	req.data[1] = 0x12345678; /* address */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_i2c_message)
{
	union request req = {0};
	struct response rsp = {0};

	/* Reset timer counter and set up the fake */
	timer_counter = 0;
	ReadReg_fake.custom_fake = ReadReg_timer_fake;

	req.data[0] = (0x1 << 24U)    /*Write Operation*/
		      | (0x50 << 16U) /* target address */
		      | (1U << 8U)    /*Line Id*/
		      | MSG_TYPE_I2C_MESSAGE;
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_ping_dm)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_PING_DM;
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_set_wdt_timeout)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_SET_WDT_TIMEOUT;
	req.data[1] = 30000; /* timeout in ms */
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_asic_state0)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_ASIC_STATE0;
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zassert_equal(rsp.data[0], 0);
}

ZTEST(msgqueue, test_msg_type_asic_state3)
{
	union request req = {0};
	struct response rsp = {0};

	req.data[0] = MSG_TYPE_ASIC_STATE3;
	msgqueue_request_push(0, &req);
	process_message_queues();
	msgqueue_response_pop(0, &rsp);

	zexpect_equal(rsp.data[0], 0);

	set_asic_state(0);
}

ZTEST_SUITE(msgqueue, NULL, NULL, NULL, NULL, NULL);
