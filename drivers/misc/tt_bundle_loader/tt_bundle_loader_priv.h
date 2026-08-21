/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TT_BUNDLE_LOADER_PRIV_H
#define TT_BUNDLE_LOADER_PRIV_H

#include <zephyr/drivers/misc/tt_bundle_loader.h>

/** @brief Get the TOC entry at the given index, or NULL if index is out of range */
const struct fw_bundle_toc_entry *tt_bundle_toc_get_entry(const struct fw_bundle_toc *toc,
							  uint64_t index);

#endif /* TT_BUNDLE_LOADER_PRIV_H */
