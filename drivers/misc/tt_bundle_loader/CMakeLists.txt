# Copyright (c) 2026 Tenstorrent AI ULC
# SPDX-License-Identifier: Apache-2.0
zephyr_library()
zephyr_library_sources_ifdef(CONFIG_TT_BUNDLE_LOADER tt_bundle_loader.c)
zephyr_library_sources_ifdef(CONFIG_TT_MMK_BUN2_LOADER tt_mmk_bun2_loader.c)
