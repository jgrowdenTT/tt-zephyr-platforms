/*
 * Copyright (c) 2024 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TENSTORRENT_MSGQUEUE_H_
#define TENSTORRENT_MSGQUEUE_H_

#include <stdint.h>

#include <zephyr/sys/iterable_sections.h>
#include <tenstorrent/msg_type.h>

#define NUM_MSG_QUEUES         4
#define MSG_QUEUE_SIZE         4
#define MSG_QUEUE_POINTER_WRAP (2 * MSG_QUEUE_SIZE)
#define REQUEST_MSG_LEN        8
#define RESPONSE_MSG_LEN       8

#define MSG_TYPE_INDEX 0
#define MSG_TYPE_MASK  0xFF
#define MSG_TYPE_SHIFT 0

#define MESSAGE_QUEUE_STATUS_MESSAGE_RECOGNIZED 0xff
#define MESSAGE_QUEUE_STATUS_SCRATCH_ONLY       0xfe

#ifdef __cplusplus
extern "C" {
#endif

struct message_queue_header {
	/* 16B for CPU writes, ARC reads */
	uint32_t request_queue_wptr;
	uint32_t response_queue_rptr;
	uint32_t unused_1;
	uint32_t unused_2;

	/* 16B for ARC writes, CPU reads */
	uint32_t request_queue_rptr;
	uint32_t response_queue_wptr;
	uint32_t last_serial;
	uint32_t unused_3;
};

/**
 * @defgroup tt_msg_apis Host Message Interface
 * @brief Interface for handling host request and response messages between the Tenstorrent host and
 * ARC processor.
 *
 * The host will send a @ref request, specifying the @ref request::command_code (of
 * type @ref tt_msg_type) SMC firmware will parse this message and send back a @ref response.
 *
 * Specific types of requests are parsed via the union members of @ref request and documented
 * therein.
 * @{
 */

/** @brief Host request to force the fan speed
 * @details Messages of this type are processed by @ref force_fan_speed
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_FORCE_FAN_SPEED*/
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief The raw speed of the fan to set, as a percentage from 0 to 100 */
	uint32_t raw_speed;
} force_fan_speed_rqst_t;

/** @brief Host request to set voltage
 * @details Messages of this type are processed by @ref set_voltage_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_SET_VOLTAGE */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief I2C slave address (P0V8_VCORE_ADDR or P0V8_VCOREM_ADDR) */
	uint32_t slave_addr;

	/** @brief Voltage to set in millivolts */
	uint32_t voltage_in_mv;
} set_voltage_rqst_t;

/** @brief Host request to get voltage
 * @details Messages of this type are processed by @ref get_voltage_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_GET_VOLTAGE */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief I2C slave address (P0V8_VCORE_ADDR or P0V8_VCOREM_ADDR) */
	uint32_t slave_addr;
} get_voltage_rqst_t;

/** @brief Host request to switch VOUT control
 * @details Messages of this type are processed by @ref switch_vout_control_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_SWITCH_VOUT_CONTROL */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief VOUT control source */
	uint32_t source;
} switch_vout_control_rqst_t;

/** @brief Host request to switch clock scheme
 * @details Messages of this type are processed by @ref switch_clk_scheme_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_SWITCH_CLK_SCHEME */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Clock scheme to switch to */
	uint32_t scheme;
} switch_clk_scheme_rqst_t;

/** @brief Host request for debug NOC translation
 * @details Messages of this type are processed by @ref DebugNocTranslationHandler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_DEBUG_NOC_TRANSLATION */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief NOC translation parameters (implementation specific) */
	uint32_t params[7];
} debug_noc_translation_rqst_t;

/** @brief Host request to send PCIe MSI
 * @details Messages of this type are processed by @ref send_pcie_msi_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_SEND_PCIE_MSI */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief MSI parameters (implementation specific) */
	uint32_t params[7];
} send_pcie_msi_rqst_t;

/** @brief Host request to read EEPROM
 * @details Messages of this type are processed by @ref read_eeprom_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_READ_EEPROM */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief EEPROM read parameters (implementation specific) */
	uint32_t params[7];
} read_eeprom_rqst_t;

/** @brief Host request to write EEPROM
 * @details Messages of this type are processed by @ref write_eeprom_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_WRITE_EEPROM */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief EEPROM write parameters (implementation specific) */
	uint32_t params[7];
} write_eeprom_rqst_t;

/** @brief Host request to read temperature sensor
 * @details Messages of this type are processed by @ref read_ts_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_READ_TS */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Temperature sensor ID */
	uint32_t id;
} read_ts_rqst_t;

/** @brief Host request to read phase detector
 * @details Messages of this type are processed by @ref read_pd_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_READ_PD */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Delay chain setting */
	uint32_t delay_chain;

	/** @brief Phase detector ID */
	uint32_t id;
} read_pd_rqst_t;

/** @brief Host request to read voltage monitor
 * @details Messages of this type are processed by @ref read_vm_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_READ_VM */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Voltage monitor ID */
	uint32_t id;
} read_vm_rqst_t;

/** @brief Host request for I2C message transaction
 * @details Messages of this type are processed by @ref i2c_message_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_I2C_MESSAGE */
	uint8_t command_code;

	/** @brief I2C master ID */
	uint8_t i2c_mst_id;

	/** @brief I2C slave address (7-bit) */
	uint8_t i2c_slave_address;

	/** @brief Number of bytes to write */
	uint8_t num_write_bytes;

	/** @brief Number of bytes to read */
	uint8_t num_read_bytes;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Write data buffer (up to 24 bytes) */
	uint8_t write_data[24];
} i2c_message_rqst_t;

/** @brief Host request to reinitialize Tensix
 * @details Messages of this type are processed by @ref ReinitTensix
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_REINIT_TENSIX */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Tensix reinit parameters (implementation specific) */
	uint32_t params[7];
} reinit_tensix_rqst_t;

/** @brief Host request to start AI sweep
 * @details Messages of this type are processed by @ref SweepAiclkHandler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_AISWEEP_START */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Sweep low frequency */
	uint32_t sweep_low;

	/** @brief Sweep high frequency */
	uint32_t sweep_high;
} aisweep_start_rqst_t;

/** @brief Host request to stop AI sweep
 * @details Messages of this type are processed by @ref SweepAiclkHandler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_AISWEEP_STOP */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];
} aisweep_stop_rqst_t;

/** @brief Host request to force AI clock
 * @details Messages of this type are processed by @ref ForceAiclkHandler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_FORCE_AICLK */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Forced frequency */
	uint32_t forced_freq;
} force_aiclk_rqst_t;

/** @brief Host request to get AI clock
 * @details Messages of this type are processed by @ref get_aiclk_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_GET_AICLK */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];
} get_aiclk_rqst_t;

/** @brief Host request to force VDD
 * @details Messages of this type are processed by @ref ForceVddHandler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_FORCE_VDD */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Forced voltage */
	uint32_t forced_voltage;
} force_vdd_rqst_t;

/** @brief Host request for AI clock go busy
 * @details Messages of this type are processed by @ref AiclkBusyHandler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_AICLK_GO_BUSY */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];
} aiclk_go_busy_rqst_t;

/** @brief Host request for AI clock go long idle
 * @details Messages of this type are processed by @ref AiclkBusyHandler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_AICLK_GO_LONG_IDLE */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];
} aiclk_go_long_idle_rqst_t;

/** @brief Host request to trigger reset
 * @details Messages of this type are processed by @ref reset_dm_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_TRIGGER_RESET */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Reset parameters (implementation specific) */
	uint32_t params[7];
} trigger_reset_rqst_t;

/** @brief Host request for PCIe DMA chip to host transfer
 * @details Messages of this type are processed by @ref pcie_dma_transfer_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_PCIE_DMA_CHIP_TO_HOST_TRANSFER */
	uint8_t command_code;

	/** @brief Completion data */
	uint8_t completion_data;

	/** @brief Two bytes of padding */
	uint8_t pad[2];

	/** @brief Transfer size in bytes */
	uint32_t transfer_size_bytes;

	/** @brief Chip address (low 32 bits) */
	uint32_t chip_addr_low;

	/** @brief Chip address (high 32 bits) */
	uint32_t chip_addr_high;

	/** @brief Host address (low 32 bits) */
	uint32_t host_addr_low;

	/** @brief Host address (high 32 bits) */
	uint32_t host_addr_high;

	/** @brief MSI completion address (low 32 bits) */
	uint32_t msi_completion_addr_low;

	/** @brief MSI completion address (high 32 bits) */
	uint32_t msi_completion_addr_high;
} pcie_dma_chip_to_host_transfer_rqst_t;

/** @brief Host request for PCIe DMA host to chip transfer
 * @details Messages of this type are processed by @ref pcie_dma_transfer_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_PCIE_DMA_HOST_TO_CHIP_TRANSFER */
	uint8_t command_code;

	/** @brief Completion data */
	uint8_t completion_data;

	/** @brief Two bytes of padding */
	uint8_t pad[2];

	/** @brief Transfer size in bytes */
	uint32_t transfer_size_bytes;

	/** @brief Chip address (low 32 bits) */
	uint32_t chip_addr_low;

	/** @brief Chip address (high 32 bits) */
	uint32_t chip_addr_high;

	/** @brief Host address (low 32 bits) */
	uint32_t host_addr_low;

	/** @brief Host address (high 32 bits) */
	uint32_t host_addr_high;

	/** @brief MSI completion address (low 32 bits) */
	uint32_t msi_completion_addr_low;

	/** @brief MSI completion address (high 32 bits) */
	uint32_t msi_completion_addr_high;
} pcie_dma_host_to_chip_transfer_rqst_t;

/** @brief Host request for ASIC state 0
 * @details Messages of this type are processed by @ref asic_state_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_ASIC_STATE0 */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief ASIC state parameters (implementation specific) */
	uint32_t params[7];
} asic_state0_rqst_t;

/** @brief Host request for ASIC state 3
 * @details Messages of this type are processed by @ref asic_state_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_ASIC_STATE3 */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief ASIC state parameters (implementation specific) */
	uint32_t params[7];
} asic_state3_rqst_t;

/** @brief Host request to toggle Tensix reset
 * @details Messages of this type are processed by @ref ToggleTensixReset
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_TOGGLE_TENSIX_RESET */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Tensix reset parameters (implementation specific) */
	uint32_t params[7];
} toggle_tensix_reset_rqst_t;

/** @brief Host request to ping data mover
 * @details Messages of this type are processed by @ref ping_dm_handler
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_PING_DM */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Ping parameters (implementation specific) */
	uint32_t params[7];
} ping_dm_rqst_t;

/** @brief Host request to set watchdog timeout
 * @details Messages of this type are processed by @ref set_watchdog_timeout
 */
typedef struct {
	/** @brief The command code corresponding to @ref MSG_TYPE_SET_WDT_TIMEOUT */
	uint8_t command_code;

	/** @brief Three bytes of padding */
	uint8_t pad[3];

	/** @brief Watchdog timeout parameters (implementation specific) */
	uint32_t params[7];
} set_wdt_timeout_rqst_t;

/** @brief A tenstorrent host request*/
union request {
	/** @brief The interpretation of the request as an array of uint32_t entries*/
	uint32_t data[REQUEST_MSG_LEN];

	/** @brief The interpretation of the request as just the first byte representing command
	 * code
	 */
	uint8_t command_code;

	/** @brief A force fan speed request*/
	force_fan_speed_rqst_t force_fan_speed;

	/** @brief A set voltage request */
	set_voltage_rqst_t set_voltage;

	/** @brief A get voltage request */
	get_voltage_rqst_t get_voltage;

	/** @brief A switch VOUT control request */
	switch_vout_control_rqst_t switch_vout_control;

	/** @brief A switch clock scheme request */
	switch_clk_scheme_rqst_t switch_clk_scheme;

	/** @brief A debug NOC translation request */
	debug_noc_translation_rqst_t debug_noc_translation;

	/** @brief A send PCIe MSI request */
	send_pcie_msi_rqst_t send_pcie_msi;

	/** @brief A read EEPROM request */
	read_eeprom_rqst_t read_eeprom;

	/** @brief A write EEPROM request */
	write_eeprom_rqst_t write_eeprom;

	/** @brief A read temperature sensor request */
	read_ts_rqst_t read_ts;

	/** @brief A read phase detector request */
	read_pd_rqst_t read_pd;

	/** @brief A read voltage monitor request */
	read_vm_rqst_t read_vm;

	/** @brief An I2C message request */
	i2c_message_rqst_t i2c_message;

	/** @brief A reinitialize Tensix request */
	reinit_tensix_rqst_t reinit_tensix;

	/** @brief An AI sweep start request */
	aisweep_start_rqst_t aisweep_start;

	/** @brief An AI sweep stop request */
	aisweep_stop_rqst_t aisweep_stop;

	/** @brief A force AI clock request */
	force_aiclk_rqst_t force_aiclk;

	/** @brief A get AI clock request */
	get_aiclk_rqst_t get_aiclk;

	/** @brief A force VDD request */
	force_vdd_rqst_t force_vdd;

	/** @brief An AI clock go busy request */
	aiclk_go_busy_rqst_t aiclk_go_busy;

	/** @brief An AI clock go long idle request */
	aiclk_go_long_idle_rqst_t aiclk_go_long_idle;

	/** @brief A trigger reset request */
	trigger_reset_rqst_t trigger_reset;

	/** @brief A PCIe DMA chip to host transfer request */
	pcie_dma_chip_to_host_transfer_rqst_t pcie_dma_chip_to_host_transfer;

	/** @brief A PCIe DMA host to chip transfer request */
	pcie_dma_host_to_chip_transfer_rqst_t pcie_dma_host_to_chip_transfer;

	/** @brief An ASIC state 0 request */
	asic_state0_rqst_t asic_state0;

	/** @brief An ASIC state 3 request */
	asic_state3_rqst_t asic_state3;

	/** @brief A toggle Tensix reset request */
	toggle_tensix_reset_rqst_t toggle_tensix_reset;

	/** @brief A ping data mover request */
	ping_dm_rqst_t ping_dm;

	/** @brief A set watchdog timeout request */
	set_wdt_timeout_rqst_t set_wdt_timeout;
};

/** @} */

struct response {
	uint32_t data[RESPONSE_MSG_LEN];
};

typedef uint8_t (*msgqueue_request_handler_t)(const union request *req, struct response *rsp);

struct msgqueue_handler {
	uint32_t msg_type;
	msgqueue_request_handler_t handler;
};

#define REGISTER_MESSAGE(msg, func)                                                                \
	const STRUCT_SECTION_ITERABLE(msgqueue_handler, registration_for_##msg) = {                \
		.msg_type = msg,                                                                   \
		.handler = func,                                                                   \
	}

void process_message_queues(void);
void msgqueue_register_handler(uint32_t msg_code, msgqueue_request_handler_t handler);

int msgqueue_request_push(uint32_t msgqueue_id, const union request *request);
int msgqueue_request_pop(uint32_t msgqueue_id, union request *request);
int msgqueue_response_push(uint32_t msgqueue_id, const struct response *response);
int msgqueue_response_pop(uint32_t msgqueue_id, struct response *response);
void init_msgqueue(void);

#ifdef __cplusplus
}
#endif

#endif
