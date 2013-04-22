#include "LPC13xx.h"
#include "buttons.h"
#include "math_utils.h"
#include "logger.h"
#include "gpio.h"

//#define DEBOUNCE_CONSTANT 10
#define DEBOUNCE_CONSTANT 5

uint32_t buttons_msticks[] = {0, 0, 0, 0};
uint32_t buttons_diff[] = {0, 0, 0, 0};
uint32_t buttons_oldState = 0;
uint32_t buttons_newState = 0;
uint32_t buttons_inputs[] = {BUTTON0, BUTTON1, BUTTON2, BUTTON3};
uint8_t buttons_input_count = sizeof(buttons_inputs) / sizeof(uint32_t);


void buttons_init(void) {
	// no need to init
	// input and pull-up is default

	GPIOSetDir( PORT0, BUTTON0, 0 );
	GPIOSetDir( PORT0, BUTTON1, 0 );
	GPIOSetDir( PORT0, BUTTON2, 0 );
	GPIOSetDir( PORT0, BUTTON3, 0 );
}

uint32_t buttons_read_status() {
	// we're are using GPIO0 here !!!
	// note that data is inverted (logic 0 -> 1) since we're are using pull-ups
	return ~LPC_GPIO0->DATA & (BUTTON0 | BUTTON1 | BUTTON2 | BUTTON3 );
}

/**
 * process buttons with DEBOUNCE
 */
void buttons_process(uint32_t msticks) {
	uint8_t i;
	uint32_t d;

	for(i = 0; i < buttons_input_count; i++) {
		if (buttons_msticks[i] != 0) {
			d = math_calc_diff(msticks, buttons_msticks[i]);
			if (d > DEBOUNCE_CONSTANT) {
				buttons_diff[i] = d;
				buttons_msticks[i] = 0;
			}
		}
	}

	buttons_newState = buttons_read_status();
	if (buttons_newState != buttons_oldState) {
		for(i = 0; i < buttons_input_count; i++) {
			if (buttons_newState & buttons_inputs[i]) {
				// 0 to 1 transition
				if ((buttons_oldState & buttons_inputs[i]) == 0) {
					buttons_msticks[i] = msticks;
					buttons_diff[i] = 0;
				}
			} else {
				// 1 to 0 transition
				if (buttons_oldState & buttons_inputs[i]) {
					buttons_msticks[i] = 0;
				}
			}
		}
		buttons_oldState = buttons_newState;
	}
}

uint32_t buttons_triggered(uint8_t index) {
	if (buttons_diff[index] > 1) {
		buttons_diff[index] = 0;
		return 1;
	}
	return 0;
}
