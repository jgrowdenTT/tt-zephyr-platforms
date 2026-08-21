/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define DT_DRV_COMPAT tenstorrent_bundle_loader

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/misc/tt_bundle_loader.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/sys_io.h>

#include "tt_bundle_loader_priv.h"

BUILD_ASSERT(sizeof(struct fw_bundle_manifest) == 1184, "fw_bundle_manifest must be 1184 bytes");
LOG_MODULE_REGISTER(tt_bundle_loader, CONFIG_TT_BUNDLE_LOADER_LOG_LEVEL);

const struct fw_bundle_toc_entry *tt_bundle_toc_get_entry(const struct fw_bundle_toc *toc,
							  uint64_t index)
{
	if (index >= toc->image_count) {
		return NULL;
	}

	return &toc->entries[index];
}
