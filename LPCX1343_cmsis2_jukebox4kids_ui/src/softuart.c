#include "LPC13xx.h"
#include "gpio.h"
#include "softuart.h"
#include "logger.h"
#include "math_utils.h"
#include "queue.h"
#include "logger.h"

uint8_t softuart_buffer_data[SOFTUART_BUFFER_SIZE];
ringbuffer_t softuart_rbuffer = {.buffer=softuart_buffer_data, .head=0, .tail=0, .count=0, .size=SOFTUART_BUFFER_SIZE};

uint8_t softuart_current_bit = 0; // 0 is start bit
uint8_t softuart_current_byte = 0;
uint8_t softuart_error_reading = 0;
trigger_config_t *trigger_config = 0;
uint8_t softuart_is_start_triggered = 0;
uint8_t softuart_is_stop_triggered = 0;

void softuart_init() {
	GPIOSetDir( SOFTUART_PORT, SOFTUART_RX_PIN, 0 );
	/* port0_1, single edge trigger on falling edge*/
	GPIOSetInterrupt( SOFTUART_PORT, SOFTUART_RX_PIN, 0, 0, 0 );

    NVIC_EnableIRQ(TIMER_16_0_IRQn);
}

void softuart_clear() {
	softuart_current_bit = 0;
	softuart_current_byte = 0;
	softuart_error_reading = 0;
}

void softuart_start_sampling() {
	// disable ext interrupt
	softuart_disable();
	softuart_clear();
	// prepare timer0 for reading start bit
    LPC_TMR16B0->TCR = 0x02;		/* reset timer */
    LPC_TMR16B0->PR  = 0x00;		/* set prescaler to zero */
    LPC_TMR16B0->MR0 = 52 * ((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/ 1000000);
    LPC_TMR16B0->MCR = 0x05; /* enable interrupt and stop timer on match */
    LPC_TMR16B0->TCR = 0x01; // start timer
}


void softuart_reset() {
	queue_reset(&softuart_rbuffer);
	softuart_clear();
	softuart_is_start_triggered = 0;
	softuart_is_stop_triggered = 0;
	softuart_enable();
}

void softuart_enable() {
	// WARNING: this is crucial!!!
	// clear all pending interrupts for this pin first
	GPIOIntClear( SOFTUART_PORT, SOFTUART_RX_PIN );

	// re-enable interrupt
	GPIOIntEnable( SOFTUART_PORT, SOFTUART_RX_PIN );
}

void softuart_disable() {
	GPIOIntDisable( SOFTUART_PORT, SOFTUART_RX_PIN );
}


void softuart_set_trigger_config(trigger_config_t *triggerconf) {
	trigger_config = triggerconf;
}

uint8_t softuart_data_available() {
	return queue_dataAvailable(&softuart_rbuffer);
}

uint8_t softuart_done_receiving() {
	return softuart_is_stop_triggered;
}

uint8_t softuart_data_count() {
	return queue_count(&softuart_rbuffer);
}

uint8_t softuart_read_byte() {
	return queue_read(&softuart_rbuffer);
}

void softuart_process(uint32_t msticks) {

}

void softuart_check_add_data_to_queue(uint8_t data) {
	if (trigger_config) {

		// the order is important here!
		if (trigger_config->start_trigger_enabled) {
			if (!softuart_is_start_triggered && data == trigger_config->start_trigger) {
				softuart_is_start_triggered = 1;
			}
		}
		else {
			softuart_is_start_triggered = 1;
		}

		if (softuart_is_start_triggered && !softuart_is_stop_triggered) {
			queue_put(&softuart_rbuffer, data);
		}

		if (trigger_config->stop_trigger_enabled) {
			if (!softuart_is_stop_triggered && data == trigger_config->stop_trigger) {
				softuart_is_stop_triggered = 1;
			}
		}
		else {
			softuart_is_stop_triggered = 0;
		}
	}
	else {
		queue_put(&softuart_rbuffer, data);
	}
}


void softuart_sample() {

	// ignore start bit
	if (softuart_current_bit == 0) {
		// do nothing
	}

	// process data
	if (softuart_current_bit > 0 && softuart_current_bit < 9) {
		// process bit
		if (GPIOGetValue(SOFTUART_PORT, SOFTUART_RX_PIN)) {
			softuart_current_byte |= 1 << (softuart_current_bit - 1);
		}
	}

	// check stop bit
	if (softuart_current_bit == 9) {
		if (GPIOGetValue(SOFTUART_PORT, SOFTUART_RX_PIN)) {
			softuart_check_add_data_to_queue(softuart_current_byte);
			//logger_logNumberln(softuart_current_byte);
		}
		else {
			softuart_error_reading = 1;
		}

		// stop timer
		LPC_TMR16B0->TCR = 0x00;
		LPC_TMR16B0->MCR = 0x00;
		// re-enable ext interrupt
		softuart_enable();
	}

	// increase current bit
	softuart_current_bit++;
}

void TIMER16_0_IRQHandler(void) {
	LPC_TMR16B0->IR = 1; /* clear interrupt flag */
	if (softuart_current_bit == 0) {
		LPC_TMR16B0->TCR = 0x02; /* reset timer */
		LPC_TMR16B0->PR  = 0x00; /* set prescaler to zero */
		LPC_TMR16B0->MR0 = 104 * ((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV) / 1000000);
		LPC_TMR16B0->MCR = 0x03; /* enable interrupt and reset timer on match */
		LPC_TMR16B0->TCR = 0x01; // start timer
	}
	softuart_sample();
}

