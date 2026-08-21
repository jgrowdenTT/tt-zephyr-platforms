/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define DT_DRV_COMPAT tenstorrent_mmk_bun2_loader

#include <errno.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/misc/tt_bundle_loader.h>
#include <zephyr/init.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "tt_bundle_loader_priv.h"

LOG_MODULE_REGISTER(tt_mmk_bun2_loader, CONFIG_TT_MMK_BUN2_LOADER_LOG_LEVEL);

/* Host <-> SMC boot handshake states, written/polled via the host-boot-state scratch register */
#define HOST_BOOT_STATE_WAIT_FOR_BUNDLE (1U)
#define HOST_BOOT_STATE_BUNDLE_STAGED   (2U)

/* Bundle validation handshake bits, in the bundle-validation scratch register */
#define BUNDLE_READY_FOR_VALIDATION_BIT BIT(0)
#define BUNDLE_VALIDATED_BIT            BIT(1)

/* SMC_CPU_SMC_CPU_CTRL_RESET_VECTOR_0 local address */
#define SMC_RESET_VECTOR0_ADDR 0xC0010000UL

struct bun2_loader_config {

	uintptr_t host_boot_state_addr;
	uintptr_t bundle_validation_addr;
	uintptr_t staging_area;
	size_t staging_area_size;
};

static int bun2_loader_init(const struct device *dev)
{
	const struct bun2_loader_config *cfg = dev->config;

	/* A) Tell the host we are ready to receive a bundle */
	sys_write32(HOST_BOOT_STATE_WAIT_FOR_BUNDLE, cfg->host_boot_state_addr);

	/* B) Wait for the host to stage the bundle in the staging area */
	LOG_INF("Waiting for BUN2 staging");
	while (sys_read32(cfg->host_boot_state_addr) != HOST_BOOT_STATE_BUNDLE_STAGED) {
		k_busy_wait(100);
	}

	/* Close host access to the staging area; only the SMC touches it from here on */

	/* D) Request validation of the staged bundle */
	sys_write32(BUNDLE_READY_FOR_VALIDATION_BIT, cfg->bundle_validation_addr);

	/* E) Wait for validation to complete */
	LOG_INF("Waiting for BUN2 validation");
	while (!(sys_read32(cfg->bundle_validation_addr) & BUNDLE_VALIDATED_BIT)) {
		k_busy_wait(100);
	}

	const struct fw_bundle_manifest *manifest =
		(const struct fw_bundle_manifest *)cfg->staging_area;
	const struct fw_bundle_toc *toc =
		(const struct fw_bundle_toc *)(cfg->staging_area + manifest->payload_offset);

	/* MMK bundles always place the K-SMC BL1 image at TOC index 0 */
	const struct fw_bundle_toc_entry *bl1_entry = tt_bundle_toc_get_entry(toc, 0);

	if (bl1_entry == NULL || bl1_entry->type != FW_BUNDLE_IMG_TYPE_SMC_BL1) {
		LOG_ERR("No SMC BL1 image found at TOC index 0 in staged bundle");
		return -ENOENT;
	}

	/* F) Copy SMC BL1 from the staged bundle to its execute location */
	memcpy((void *)bl1_entry->load_addr,
	       (const void *)(cfg->staging_area + manifest->payload_offset + bl1_entry->offset),
	       bl1_entry->length);

	__asm__ volatile("fence\nfence.i" ::: "memory");

	LOG_INF("Jumping to SMC BL1 at 0x%llx", bl1_entry->load_addr);

	/* G) Jump to SMC BL1. Does not return; BL1 owns the core from here on. */
	/* mret gives BL1 a clean entry: M-mode, MIE=0, no BL0P5 mtvec leaking through */
	sys_write32((uint32_t)bl1_entry->entry_point, SMC_RESET_VECTOR0_ADDR);
	__asm__ volatile("csrw mepc, %0\n"
			 "li   t0, 0x1800\n" /* MPP=M-mode, MIE=0, MPIE=0 */
			 "csrw mstatus, t0\n"
			 "mret\n"
			 :
			 : "r"((uintptr_t)bl1_entry->entry_point)
			 : "t0");

	/* Unreachable unless BL1 returns, which means the handoff failed and the core's
	 * state (stack, vector table, PMP, ...) can no longer be trusted.
	 */
	LOG_ERR("SMC BL1 unexpectedly returned");
	k_panic();

	return 0;
}

#define BUN2_LOADER_INIT(inst)                                                                     \
	static const struct bun2_loader_config bun2_loader_config_##inst = {                       \
		.host_boot_state_addr = DT_REG_ADDR(DT_INST_PHANDLE(inst, host_boot_state)),       \
		.bundle_validation_addr = DT_REG_ADDR(DT_INST_PHANDLE(inst, bundle_validation)),   \
		.staging_area = DT_INST_REG_ADDR(inst),                                            \
		.staging_area_size = DT_INST_REG_SIZE(inst),                                       \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, bun2_loader_init, NULL, NULL, &bun2_loader_config_##inst,      \
			      POST_KERNEL, CONFIG_TT_MMK_BUN2_LOADER_INIT_PRIO, NULL);

DT_INST_FOREACH_STATUS_OKAY(BUN2_LOADER_INIT)
