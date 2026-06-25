/*
 * Copyright (c) 2024 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

/*
 * Memory-mapped addresses for ARC tile RESET_UNIT scratch registers
 * From tile_regs.h:
 *   ARC_APB_BASE = 0x80000000
 *   ARC_APB_RESET_UNIT_BASE = 0x00030000
 *   RESET_UNIT_SCRATCH_RAM(i) = 0x400 + 4*(i)
 * So RESET_UNIT_SCRATCH_RAM(2) = 0x80030000 + 0x400 + 8 = 0x80030408
 */
#define ARC_BOOT_STATUS_REG (volatile uint32_t *)(0x80030408)
#define UNUSED(x) (void)(x)

int main(void)
{
	uint32_t original_value, new_value, readback_value;

	/* Read the current boot status value */
	original_value = *ARC_BOOT_STATUS_REG;

	*ARC_BOOT_STATUS_REG = 0x5;

	/* Read it back to confirm */
	readback_value = *ARC_BOOT_STATUS_REG;

	/* Use the values so they don't get optimized away */
	UNUSED(original_value);
	UNUSED(readback_value);

	/* Loop forever */
	while (1) {
	}

	return 0;
}
