#include "LPC13xx.h"
#include "gpio.h"
#include "ext_int.h"
#include "barcode.h"
#include "softuart.h"

/**
 * It's important to define the correct interrupt handler here (e.g. in this example PORT0 is used)
 */
void PIOINT0_IRQHandler(void) {

	if ( GPIOIntStatus( BARCODE_PORT, BARCODE_CLK_PIN ) )  {
		barcode_sample();
		GPIOIntClear( BARCODE_PORT, BARCODE_CLK_PIN );
	}

	if ( GPIOIntStatus( SOFTUART_PORT, SOFTUART_RX_PIN ) )  {

		// start receiving bits
		softuart_start();

		GPIOIntClear( SOFTUART_PORT, SOFTUART_RX_PIN );
	}

	return;
}
