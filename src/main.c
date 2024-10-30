/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "lcd_screen_i2c.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/util.h>

#define LED_YELLOW_NODE DT_ALIAS(led_yellow)
#define LCD_I2C_NODE DT_NODELABEL(lcd_screen)

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

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
	
	while(1){
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

		// ADC 
		int err;
		uint32_t count = 0;
		uint16_t buf;
		struct adc_sequence sequence = {
			.buffer = &buf,
			/* buffer size in bytes, not number of samples */
			.buffer_size = sizeof(buf),
		};

		/* Configure channels individually prior to sampling. */
		for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
			if (!adc_is_ready_dt(&adc_channels[i])) {
				printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
				return 0;
			}

			err = adc_channel_setup_dt(&adc_channels[i]);
			if (err < 0) {
				printk("Could not setup channel #%d (%d)\n", i, err);
				return 0;
			}
		}

	#ifndef CONFIG_COVERAGE
	#else
		for (int k = 0; k < 10; k++) {
	#endif
			printk("ADC reading[%u]:\n", count++);
			for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
				int32_t val_mv;

				printk("- %s, channel %d: ",
					adc_channels[i].dev->name,
					adc_channels[i].channel_id);

				(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

				err = adc_read_dt(&adc_channels[i], &sequence);
				if (err < 0) {
					printk("Could not read (%d)\n", err);
					continue;
				}

				/*
				* If using differential mode, the 16 bit value
				* in the ADC sample buffer should be a signed 2's
				* complement value.
				*/
				if (adc_channels[i].channel_cfg.differential) {
					val_mv = (int32_t)((int16_t)buf);
				} else {
					val_mv = (int32_t)buf;
				}
				printk("%"PRId32, val_mv);
				err = adc_raw_to_millivolts_dt(&adc_channels[i],
								&val_mv);
				/* conversion to mV may not be supported, skip if not */
				if (err < 0) {
					printk(" (value in mV not available)\n");
				} else {
					printk(" = %"PRId32" mV\n", val_mv);
				}
			}
		k_sleep(K_SECONDS(2));
	}
	return 0;
	}
