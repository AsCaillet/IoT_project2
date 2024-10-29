/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "lcd_screen_i2c.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/devicetree.h>

#define LED_YELLOW_NODE DT_ALIAS(led_yellow)
#define LCD_I2C_NODE DT_NODELABEL(lcd_screen)

const struct i2c_dt_spec dev_lcd_screen = I2C_DT_SPEC_GET(LCD_I2C_NODE);
const struct gpio_dt_spec led_yellow_gpio = GPIO_DT_SPEC_GET_OR(LED_YELLOW_NODE, gpios, {0});

int main(void)
{
	// Init device
    init_lcd(&dev_lcd_screen);

    // Display a message
    write_lcd(&dev_lcd_screen, HELLO_MSG, LCD_LINE_1);
    //write_lcd_clear(&dev_lcd_screen, ZEPHYR_MSG, LCD_LINE_2);

	// Hello World
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);

	// LED ON
	gpio_pin_configure_dt(&led_yellow_gpio, GPIO_OUTPUT_HIGH);
	return 0;
}
