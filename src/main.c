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
#include <zephyr/drivers/sensor.h>

#define LED_YELLOW_NODE DT_ALIAS(led_yellow)
#define LCD_I2C_NODE DT_NODELABEL(lcd_screen)

const struct i2c_dt_spec dev_lcd_screen = I2C_DT_SPEC_GET(LCD_I2C_NODE);
const struct gpio_dt_spec led_yellow_gpio = GPIO_DT_SPEC_GET_OR(LED_YELLOW_NODE, gpios, {0});
const struct device *const dht11 = DEVICE_DT_GET_ONE(aosong_dht);

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

	// Variables pour la temp et l'hum
	struct sensor_value temperature, humidity;

	// Data capteur
	sensor_sample_fetch(dht11);

	// Obtenir temperature et humidite
	sensor_channel_get(dht11, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
	sensor_channel_get(dht11, SENSOR_CHAN_HUMIDITY, &humidity);

	// Convers° des valeurs pour l'affichage
	int temp = sensor_value_to_double(&temperature);
	int hum = sensor_value_to_double(&humidity);

	printf("Temperature = %d\n",temp);
	printf("Humidite = %d\n",hum);
}
