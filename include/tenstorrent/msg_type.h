/*
 * Copyright (c) 2024 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file msg_type.h
 * @brief Tenstorrent host command IDs
 */

#ifndef TENSTORRENT_MSG_TYPE_H_
#define TENSTORRENT_MSG_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup tt_msg_apis
 * @{
 */

/** @brief Enumeration listing the available host requests IDs the SMC can process*/
enum tt_msg_type {
	/** @brief Reserved (not supported) */
	MSG_TYPE_RESERVED_01 = 0x1,

	/** @brief No-op request (not supported) */
	MSG_TYPE_NOP = 0x11,

	/** @brief @ref set_voltage_rqst_t "Set voltage request" */
	MSG_TYPE_SET_VOLTAGE = 0x12,
	/** @brief @ref get_voltage_rqst_t "Get voltage request" */
	MSG_TYPE_GET_VOLTAGE = 0x13,
	/** @brief @ref switch_clk_scheme_rqst_t "Switch clock scheme request" */
	MSG_TYPE_SWITCH_CLK_SCHEME = 0x14,
	/** @brief @ref debug_noc_translation_rqst_t "Debug NOC translation request" */
	MSG_TYPE_DEBUG_NOC_TRANSLATION = 0x15,
	/** @brief Report scratch only request (not supported) */
	MSG_TYPE_REPORT_SCRATCH_ONLY = 0x16,
	/** @brief @ref send_pcie_msi_rqst_t "Send PCIe MSI request" */
	MSG_TYPE_SEND_PCIE_MSI = 0x17,
	/** @brief @ref switch_vout_control_rqst_t "Switch VOUT control request" */
	MSG_TYPE_SWITCH_VOUT_CONTROL = 0x18,
	/** @brief @ref read_eeprom_rqst_t "Read EEPROM request" */
	MSG_TYPE_READ_EEPROM = 0x19,
	/** @brief @ref write_eeprom_rqst_t "Write EEPROM request" */
	MSG_TYPE_WRITE_EEPROM = 0x1A,
	/** @brief @ref read_ts_rqst_t "Read temperature sensor request" */
	MSG_TYPE_READ_TS = 0x1B,
	/** @brief @ref read_pd_rqst_t "Read phase detector request" */
	MSG_TYPE_READ_PD = 0x1C,
	/** @brief @ref read_vm_rqst_t "Read voltage monitor request" */
	MSG_TYPE_READ_VM = 0x1D,
	/** @brief @ref i2c_message_rqst_t "I2C message request" */
	MSG_TYPE_I2C_MESSAGE = 0x1E,
	/** @brief eFuse burn bits request (not supported) */
	MSG_TYPE_EFUSE_BURN_BITS = 0x1F,
	/** @brief @ref reinit_tensix_rqst_t "Reinitialize Tensix request" */
	MSG_TYPE_REINIT_TENSIX = 0x20,
	/** @brief Get frequency curve from voltage request (not supported) */
	MSG_TYPE_GET_FREQ_CURVE_FROM_VOLTAGE = 0x30,
	/** @brief @ref aisweep_start_rqst_t "AI sweep start request" */
	MSG_TYPE_AISWEEP_START = 0x31,
	/** @brief @ref aisweep_stop_rqst_t "AI sweep stop request" */
	MSG_TYPE_AISWEEP_STOP = 0x32,
	/** @brief @ref force_aiclk_rqst_t "Force AI clock request" */
	MSG_TYPE_FORCE_AICLK = 0x33,
	/** @brief @ref get_aiclk_rqst_t "Get AI clock request" */
	MSG_TYPE_GET_AICLK = 0x34,
	/** @brief @ref force_vdd_rqst_t "Force VDD request" */
	MSG_TYPE_FORCE_VDD = 0x39,
	/** @brief PCIe index request (not supported) */
	MSG_TYPE_PCIE_INDEX = 0x51,
	/** @brief @ref aiclk_go_busy_rqst_t "AI clock go busy request" */
	MSG_TYPE_AICLK_GO_BUSY = 0x52,
	/** @brief @ref aiclk_go_long_idle_rqst_t "AI clock go long idle request" */
	MSG_TYPE_AICLK_GO_LONG_IDLE = 0x54,
	/** @brief @ref trigger_reset_rqst_t "Trigger reset request" - arg: 3 = ASIC + M3 reset, other values = ASIC-only reset */
	MSG_TYPE_TRIGGER_RESET = 0x56,

	/** @brief Reserved (not supported) */
	MSG_TYPE_RESERVED_60 = 0x60,
	/** @brief Test request (not supported) */
	MSG_TYPE_TEST = 0x90,
	/** @brief @ref pcie_dma_chip_to_host_transfer_rqst_t "PCIe DMA chip to host transfer request" */
	MSG_TYPE_PCIE_DMA_CHIP_TO_HOST_TRANSFER = 0x9B,
	/** @brief @ref pcie_dma_host_to_chip_transfer_rqst_t "PCIe DMA host to chip transfer request" */
	MSG_TYPE_PCIE_DMA_HOST_TO_CHIP_TRANSFER = 0x9C,
	/** @brief PCIe error count reset request (not supported) */
	MSG_TYPE_PCIE_ERROR_CNT_RESET = 0x9D,
	/** @brief Trigger IRQ request (not supported) */
	MSG_TYPE_TRIGGER_IRQ = 0x9F,
	/** @brief @ref asic_state0_rqst_t "ASIC state 0 request" */
	MSG_TYPE_ASIC_STATE0 = 0xA0,
	/** @brief ASIC state 1 request (not supported) */
	MSG_TYPE_ASIC_STATE1 = 0xA1,
	/** @brief @ref asic_state3_rqst_t "ASIC state 3 request" */
	MSG_TYPE_ASIC_STATE3 = 0xA3,
	/** @brief ASIC state 5 request (not supported) */
	MSG_TYPE_ASIC_STATE5 = 0xA5,
	/** @brief Get voltage curve from frequency request (not supported) */
	MSG_TYPE_GET_VOLTAGE_CURVE_FROM_FREQ = 0xA6,

	/** @brief @ref force_fan_speed_rqst_t "Force Fan Speed Request"*/
	MSG_TYPE_FORCE_FAN_SPEED = 0xAC,
	/** @brief Get DRAM temperature request (not supported) */
	MSG_TYPE_GET_DRAM_TEMPERATURE = 0xAD,
	/** @brief @ref toggle_tensix_reset_rqst_t "Toggle Tensix reset request" */
	MSG_TYPE_TOGGLE_TENSIX_RESET = 0xAF,
	/** @brief DRAM BIST start request (not supported) */
	MSG_TYPE_DRAM_BIST_START = 0xB0,
	/** @brief NOC write word request (not supported) - additional parameters passed in ARC_MAILBOX */
	MSG_TYPE_NOC_WRITE_WORD = 0xB1,
	/** @brief Toggle Ethernet reset request (not supported) */
	MSG_TYPE_TOGGLE_ETH_RESET = 0xB2,
	/** @brief Set DRAM refresh rate request (not supported) */
	MSG_TYPE_SET_DRAM_REFRESH_RATE = 0xB3,
	/** @brief ARC DMA request (not supported) */
	MSG_TYPE_ARC_DMA = 0xB4,
	/** @brief Test SPI request (not supported) */
	MSG_TYPE_TEST_SPI = 0xB5,
	/** @brief Current date request (not supported) */
	MSG_TYPE_CURR_DATE = 0xB7,
	/** @brief Update M3 auto reset timeout request (not supported) */
	MSG_TYPE_UPDATE_M3_AUTO_RESET_TIMEOUT = 0xBC,
	/** @brief Clear number of auto resets request (not supported) */
	MSG_TYPE_CLEAR_NUM_AUTO_RESET = 0xBD,
	/** @brief Set last serial request (not supported) */
	MSG_TYPE_SET_LAST_SERIAL = 0xBE,
	/** @brief eFuse burn request (not supported) */
	MSG_TYPE_EFUSE_BURN = 0xBF,
	/** @brief @ref ping_dm_rqst_t "Ping data mover request" */
	MSG_TYPE_PING_DM = 0xC0,
	/** @brief @ref set_wdt_timeout_rqst_t "Set watchdog timeout request" */
	MSG_TYPE_SET_WDT_TIMEOUT = 0xC1,
};

/** @} */

#ifdef __cplusplus
}
#endif

#endif
