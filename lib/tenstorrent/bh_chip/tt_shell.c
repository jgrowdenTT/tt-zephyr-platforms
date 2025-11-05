/*
 * Copyright (c) 2025 Tenstorrent AI ULC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/gpio.h>
#include <stdlib.h>

LOG_MODULE_REGISTER(tt_shell, CONFIG_LOG_DEFAULT_LEVEL);

int scandump(const struct shell *sh, size_t argc, char **argv)
{
	extern bool skip_evt_loop;
	const struct gpio_dt_spec dft_tap_sel =
		GPIO_DT_SPEC_GET(DT_CHILD(DT_NODELABEL(chip0), dft_tap_sel), gpios);
	const struct gpio_dt_spec dft_test_mode =
		GPIO_DT_SPEC_GET(DT_CHILD(DT_NODELABEL(chip0), dft_test_mode), gpios);

	const struct device *gpiox1 = DEVICE_DT_GET(DT_NODELABEL(gpiox1));
	const struct device *gpiox2 = DEVICE_DT_GET(DT_NODELABEL(gpiox2));
	/* Set GPIOs 1, 13, 35 high, others low */
	/* GPIO 1 = gpio_x1 pin 1 */
	/* GPIO 8 = gpio_x1 pin 6 */
	/* GPIO 13 = gpio_x1 pin 11 */
	/* GPIO 14 = gpio_x1 pin 12 */
	/* GPIO 32 = gpio_x2 pin 0 */
	/* GPIO 35 = gpio_x2 pin 4 (only on p300x) */
	if (strcmp(argv[1], "off") == 0) {
		shell_info(sh, "Turning scandump mode off...");

		/* from GPIO expanders */
		for (int i = 0; i < 16; i++) {
			gpio_pin_configure(gpiox1, i, GPIO_OUTPUT_INACTIVE);
			gpio_pin_configure(gpiox2, i, GPIO_OUTPUT_INACTIVE);
		}

		/* directly connected to STM32 */
		gpio_pin_set_dt(&dft_tap_sel, 0);
		gpio_pin_set_dt(&dft_test_mode, 0);
		gpio_pin_configure_dt(&dft_tap_sel, GPIO_OUTPUT_INACTIVE);
		gpio_pin_configure_dt(&dft_test_mode, GPIO_OUTPUT_INACTIVE);
		skip_evt_loop = false;
		shell_info(sh, "Done!");
	} else if (strcmp(argv[1], "on") == 0) {
		shell_info(sh, "Turning scandump mode on...");
		skip_evt_loop = true;
		/* directly connected to STM32 */
		gpio_pin_configure_dt(&dft_tap_sel, GPIO_OUTPUT_HIGH);
		gpio_pin_configure_dt(&dft_test_mode, GPIO_OUTPUT_HIGH);

		k_busy_wait(100);
		/* iForcePort [list "BP_GPIO_1" "BP_GPIO_8" "BP_GPIO_13" "BP_GPIO_14" "BP_GPIO_32"
		 * "BP_GPIO_35" "DFT_TEST_MODE" "DFT_TAP_SEL"] 0b10100111
		 */

		for (int i = 0; i < 16; i++) {
			gpio_pin_configure(gpiox1, i, GPIO_OUTPUT_LOW);
			gpio_pin_configure(gpiox2, i, GPIO_OUTPUT_LOW);
		}

		/* gpio_pin_configure(gpiox1, 1, GPIO_OUTPUT_HIGH);  BH GPIO1 */
		/* gpio_pin_configure(gpiox1, 11, GPIO_OUTPUT_HIGH); BH GPIO13 */
		shell_info(sh, "Done!");
	} else {
		shell_error(sh, "Invalid MRISC power setting");
	}
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_tt_commands,
			       SHELL_CMD_ARG(scandump, NULL, "[off|on]", scandump, 2, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(tt, &sub_tt_commands, "Tensorrent commands", NULL);
