# SPDX-License-Identifier: Apache-2.0

if(NOT CONFIG_BOARD_TT_BH_SIM_MGMT)
	return()
endif()

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_UART_TT_VIRT src/vuart.c)
zephyr_library_include_directories(${ZEPHYR_TT_SYSTEM_FIRMWARE_MODULE_DIR}/lib/tenstorrent/bh_arc)
