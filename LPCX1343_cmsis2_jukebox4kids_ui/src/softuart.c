#include "LPC13xx.h"
#include "gpio.h"
#include "softuart.h"
#include "logger.h"
#include "math_utils.h"
#include "queue.h"


uint8_t softuart_buffer_data[SOFTUART_BUFFER_SIZE];
ringbuffer_t softuart_rbuffer = {.buffer=softuart_buffer_data, .head=0, .tail=0, .count=0, .size=SOFTUART_BUFFER_SIZE};

uint8_t softuart_current_bit = 0; // 0 is start bit
uint8_t softuart_current_byte = 0;
uint8_t softuart_data_available = 0;
uint8_t softuart_error_reading = 0;

void softuart_init() {
	GPIOSetDir( SOFTUART_PORT, SOFTUART_RX_PIN, 0 );
	/* port0_1, single edge trigger on falling edge*/
	GPIOSetInterrupt( SOFTUART_PORT, SOFTUART_RX_PIN, 0, 0, 0 );
}



void softuart_clear() {
	softuart_current_bit = 0;
	softuart_current_byte = 0;
	softuart_data_available = 0;
	softuart_error_reading = 0;
	// stop timer
    LPC_TMR16B0->TCR = 0x00;
}

void softuart_start() {
	// disable ext interrupt
	softuart_disable();
	softuart_clear();
	// prepare timer0 for reading start bit
    LPC_TMR16B0->TCR = 0x02;		/* reset timer */
    LPC_TMR16B0->PR  = 0x00;		/* set prescaler to zero */
    LPC_TMR16B0->MR0 = 52 * ((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/ 1000000);
    LPC_TMR16B0->MCR = 0x04; /* enable interrupt and stop timer on match */
    LPC_TMR16B0->TCR = 0x01; // start timer
}


void softuart_reset() {
	queue_reset(&softuart_rbuffer);
	softuart_clear();
}

void softuart_enable() {
	GPIOIntEnable( SOFTUART_PORT, SOFTUART_RX_PIN );
}

void softuart_disable() {
	GPIOIntDisable( SOFTUART_PORT, SOFTUART_RX_PIN );
}

void softuart_process(uint32_t msticks) {

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
			softuart_current_byte |= 1 << softuart_current_bit;
		}
	}

	// check stop bit
	if (softuart_current_bit == 9) {
		if (GPIOGetValue(SOFTUART_PORT, SOFTUART_RX_PIN)) {
			queue_put(&softuart_rbuffer, softuart_current_byte);
			softuart_clear();
			softuart_data_available = 1;

			// stop timer
			LPC_TMR16B0->TCR = 0x00;

			// re-enable ext interrupt
			softuart_enable();
		}
		else {
			softuart_error_reading = 1;
			softuart_clear();
		}
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

