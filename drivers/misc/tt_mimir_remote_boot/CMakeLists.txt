# Copyright (c) 2026 Tenstorrent AI ULC
# SPDX-License-Identifier: Apache-2.0
zephyr_library()
zephyr_library_sources_ifdef(CONFIG_TT_MIMIR_REMOTE_BOOT tt_mimir_remote_boot.c)
