/*
 * Copyright (c) 2024 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "status_reg.h"

#include <stddef.h>
#include <stdint.h>

#include <tenstorrent/uart_tt_virt.h>
#include <zephyr/kernel.h>

void uart_tt_virt_init_callback(const struct device *dev, size_t inst)
{
	volatile uint32_t *reg = (volatile uint32_t *)(uintptr_t)STATUS_FW_VUART_REG_ADDR(inst);
	*reg = (uint32_t)(uintptr_t)uart_tt_virt_get(dev);
}