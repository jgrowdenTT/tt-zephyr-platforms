/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include <zephyr/sys/util.h>

#include "status_reg.h"

#define SCRATCH_REG_COUNT 8U
#define SCRATCH_RAM_REG_COUNT 64U

#if defined(CONFIG_BOARD_TT_BH_SIM_MGMT)

static bool addr_to_scratch_index(uint32_t addr, uint32_t *index)
{
	if (addr < RESET_UNIT_SCRATCH_BASE_ADDR) {
		return false;
	}

	uint32_t offset = addr - RESET_UNIT_SCRATCH_BASE_ADDR;
	if ((offset % sizeof(uint32_t)) != 0U) {
		return false;
	}

	uint32_t idx = offset / sizeof(uint32_t);
	if (idx >= SCRATCH_REG_COUNT) {
		return false;
	}

	*index = idx;
	return true;
}

static bool addr_to_scratch_ram_index(uint32_t addr, uint32_t *index)
{
	if (addr < RESET_UNIT_SCRATCH_RAM_BASE_ADDR) {
		return false;
	}

	uint32_t offset = addr - RESET_UNIT_SCRATCH_RAM_BASE_ADDR;
	if ((offset % sizeof(uint32_t)) != 0U) {
		return false;
	}

	uint32_t idx = offset / sizeof(uint32_t);
	if (idx >= SCRATCH_RAM_REG_COUNT) {
		return false;
	}

	*index = idx;
	return true;
}

__weak uint32_t ReadReg(uint32_t addr)
{
	uint32_t idx;

	if (addr_to_scratch_index(addr, &idx)) {
		ARG_UNUSED(idx);
		return *(volatile uint32_t *)(uintptr_t)addr;
	}

	if (addr_to_scratch_ram_index(addr, &idx)) {
		ARG_UNUSED(idx);
		return *(volatile uint32_t *)(uintptr_t)addr;
	}

	return 0U;
}

__weak void WriteReg(uint32_t addr, uint32_t val)
{
	uint32_t idx;

	if (addr_to_scratch_index(addr, &idx)) {
		ARG_UNUSED(idx);
		*(volatile uint32_t *)(uintptr_t)addr = val;
		return;
	}

	if (addr_to_scratch_ram_index(addr, &idx)) {
		ARG_UNUSED(idx);
		*(volatile uint32_t *)(uintptr_t)addr = val;
		return;
	}

	ARG_UNUSED(val);
}

#else

__weak uint32_t ReadReg(uint32_t addr)
{
	ARG_UNUSED(addr);
	return 0U;
}

__weak void WriteReg(uint32_t addr, uint32_t val)
{
	ARG_UNUSED(addr);
	ARG_UNUSED(val);
}

#endif
