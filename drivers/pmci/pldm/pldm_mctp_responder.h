/*
 * Copyright (c) 2026 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TT_PMCI_PLDM_MCTP_RESPONDER_H_
#define TT_PMCI_PLDM_MCTP_RESPONDER_H_

#include <zephyr/device.h>

/**
 * @brief Return the OEM handler device associated with a PLDM MCTP responder.
 *
 * @param dev  A tenstorrent,pldm-mctp-responder device instance.
 * @return     The linked tenstorrent,pldm-oem-handler device, or NULL if none.
 */
const struct device *tt_pldm_mctp_responder_oem_handler_get(const struct device *dev);

#endif /* TT_PMCI_PLDM_MCTP_RESPONDER_H_ */
