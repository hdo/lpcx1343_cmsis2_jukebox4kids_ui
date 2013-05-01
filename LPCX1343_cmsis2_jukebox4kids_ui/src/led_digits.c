#include "LPC13xx.h"
#include "gpio.h"
#include "led_digits.h"
#include "logger.h"
#include "math_utils.h"
#include "timer16.h" // see LPC13xx_cmsis2_lib

volatile uint8_t led_digits_current_value = 0;
uint8_t led_digits_initialized = 0;
uint8_t led_digits_blink_enabled = 0;
uint8_t led_digits_blink_status = 0;
uint32_t led_blink_msticks = 0;
uint32_t led_digits_msticks = 0;
uint16_t led_digits_blink_interval = 20; // interval in 10ms
uint8_t led_green_blink_enabled = 0;
uint8_t led_red_blink_enabled = 0;

void led_digits_init() {
   // set PIO_1_0 - PIO_1_3 functions
   LPC_IOCON->R_PIO1_0 &= ~0x07;
   LPC_IOCON->R_PIO1_0 |= 0x01; /* GPIO */

   LPC_IOCON->R_PIO1_1 &= ~0x07;
   LPC_IOCON->R_PIO1_1 |= 0x01; /* GPIO */

   LPC_IOCON->R_PIO1_2 &= ~0x07;
   LPC_IOCON->R_PIO1_2 |= 0x01; /* GPIO */

   LPC_IOCON->SWDIO_PIO1_3 &= ~0x07;
   LPC_IOCON->SWDIO_PIO1_3 |= 0x01; /* GPIO */

   LPC_IOCON->R_PIO0_11 &= ~0x07;
   LPC_IOCON->R_PIO0_11 |= 0x01; /* GPIO */

   GPIOSetDir( LED_DIGITS_PORT, LED_DIGITS_PIN_A, 1 );
   GPIOSetDir( LED_DIGITS_PORT, LED_DIGITS_PIN_B, 1 );
   GPIOSetDir( LED_DIGITS_PORT, LED_DIGITS_PIN_C, 1 );
   GPIOSetDir( LED_DIGITS_PORT, LED_DIGITS_PIN_D, 1 );
   GPIOSetDir( LED_DIGITS_PORT, LED_DIGITS_PIN_L0, 1 );
   GPIOSetDir( LED_DIGITS_PORT, LED_DIGITS_PIN_L1, 1 );
   GPIOSetDir( LED_DGITIS_PORT_BL, LED_DIGITS_PIN_BL, 1);

   GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_L0, 1 );
   GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_L1, 1 );

   GPIOSetDir( LED_RED_PORT, LED_RED_PIN, 1);
   GPIOSetValue( LED_RED_PORT, LED_RED_PIN, 0);
   GPIOSetDir( LED_GREEN_PORT, LED_GREEN_PIN, 1);
   GPIOSetValue( LED_GREEN_PORT, LED_GREEN_PIN, 0);

   // set
   GPIOSetValue( LED_DGITIS_PORT_BL, LED_DIGITS_PIN_BL, 1);

   led_digits_initialized = 1;
}


void led_digits_set_digit_helper(uint8_t value) {
	GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_A, value & 0x01 );
	GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_B, (value >> 1) & 0x01 );
	GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_C, (value >> 2) & 0x01 );
	GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_D, (value >> 3) & 0x01 );
}

void led_digits_set_digit0(uint8_t value) {
	if (led_digits_initialized) {
		GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_L1, 1 );
		GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_L0, 0 );

		led_digits_set_digit_helper(value);
		delay_microseconds(0,5);

		GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_L0, 1 );
	}
}

void led_digits_set_digit1(uint8_t value) {
	if (led_digits_initialized) {
		GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_L0, 1 );
		GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_L1, 0 );

		led_digits_set_digit_helper(value);
		delay_microseconds(0,5);

		GPIOSetValue( LED_DIGITS_PORT, LED_DIGITS_PIN_L1, 1 );
	}
}

void led_digits_set_value(uint8_t value) {
	led_digits_current_value = value;
	led_digits_set_digit0(value % 10);
	led_digits_set_digit1(value / 10);
}


void led_digits_enable() {
	GPIOSetValue( LED_DGITIS_PORT_BL, LED_DIGITS_PIN_BL, 1);
}

void led_digits_disable() {
	GPIOSetValue( LED_DGITIS_PORT_BL, LED_DIGITS_PIN_BL, 0);
}

void led_digits_set_value_by_chars(uint8_t ch0, uint8_t ch1) {
	uint8_t d0 = 10; // don't show any (see CD4511 datasheet)
	uint8_t d1 = 10;
	if (ch0 >= '0' && ch0 <= '9') {
		d0 = ch0 - '0';
	}
	if (ch1 >= '0' && ch1 <= '9') {
		d1 = ch1 - '0';
	}
	led_digits_current_value = d1 * 10 + d0;
	led_digits_set_digit0(d0);
	led_digits_set_digit1(d1);
}

void led_digits_set_blink(uint8_t enabled) {
	led_digits_blink_enabled = enabled;
}

void led_digits_set_blink_interval(uint16_t interval) {
	if (interval > 1 && interval < 500) {
		led_digits_blink_interval = interval * 10;
	}
}

void led_red_set(uint8_t enabled) {
	GPIOSetValue( LED_RED_PORT, LED_RED_PIN, enabled);
}

void led_green_set(uint8_t enabled) {
	GPIOSetValue( LED_GREEN_PORT, LED_GREEN_PIN, enabled);
}

void led_red_set_blink(uint8_t enabled) {
	led_red_blink_enabled = enabled;
}

void led_green_set_blink(uint8_t enabled) {
	led_green_blink_enabled = enabled;
}

void led_red_toggle() {
	if (GPIOGetValue( LED_RED_PORT, LED_RED_PIN )) {
		GPIOSetValue( LED_RED_PORT, LED_RED_PIN, 0);
	}
	else {
		GPIOSetValue( LED_RED_PORT, LED_RED_PIN, 1);
	}
}

void led_green_toggle() {
	if (GPIOGetValue( LED_GREEN_PORT, LED_GREEN_PIN )) {
		GPIOSetValue( LED_GREEN_PORT, LED_GREEN_PIN, 0);
	}
	else {
		GPIOSetValue( LED_GREEN_PORT, LED_GREEN_PIN, 1);
	}
}

void led_digits_process(uint32_t msticks) {
	led_digits_msticks = msticks;
	if (math_calc_diff(led_digits_msticks, led_blink_msticks) > led_digits_blink_interval) {
		led_blink_msticks = led_digits_msticks;
		if (led_digits_blink_enabled) {
			if (led_digits_blink_status) {
				led_digits_blink_status = 0;
				led_digits_enable();
			}
			else {
				led_digits_blink_status = 1;
				led_digits_disable();
			}
		}
		if (led_red_blink_enabled) {
			led_red_toggle();
		}
		if (led_green_blink_enabled) {
			led_green_toggle();
		}
	}

}

