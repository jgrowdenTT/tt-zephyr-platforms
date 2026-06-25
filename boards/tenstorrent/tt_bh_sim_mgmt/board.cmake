# SPDX-License-Identifier: Apache-2.0

set(SUPPORTED_EMU_PLATFORMS qemu)

set(QEMU_binary_suffix riscv64)
set(QEMU_CPU_TYPE_${ARCH} rv64)
set(QEMU_FLAGS_${ARCH}
  -machine virt
  -bios none
  -m 256
)

include(${ZEPHYR_BASE}/boards/common/qemu.board.cmake)
