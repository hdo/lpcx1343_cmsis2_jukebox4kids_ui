#include "LPC13xx.h"
#include "gpio.h"
#include "barcode.h"
#include "uart.h"
#include "logger.h"
#include "buttons.h"
#include "led_digits.h"
#include "timer16.h"
#include "timer32.h"

#include <cr_section_macros.h>
#include <NXP/crp.h>

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

// LPCXpresso processor card LED
#define LED_PORT 0		// Port for led
#define LED_BIT 7		// Bit on port for led
#define LED_ON 1		// Level to set port to turn on led
#define LED_OFF 0		// Level to set port to turn off led

unsigned int LEDvalue = LED_OFF;

extern volatile uint32_t UARTStatus;
extern volatile uint8_t UARTTxEmpty;
extern volatile uint8_t UARTBuffer[BUFSIZE];
extern volatile uint32_t UARTCount;

extern volatile uint8_t barcode_data[BARCODE_BUFFER_SIZE];
extern volatile uint8_t barcode_current_byte;
extern volatile uint8_t barcode_data_count;


volatile uint32_t msTicks;                          /* counts 10ms timeTicks */
/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
  msTicks++;                        /* increment counter necessary in Delay() */
}


void init_timers() {
	// enabled clk for timer16_0
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);
	// enabled clk for timer16_1
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<8);
	// enabled clk for timer32_0
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);
	// enabled clk for timer32_1
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<10);
}

/*****************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void) {

   /* Setup SysTick Timer for 10 msec interrupts  */
   if (SysTick_Config(SystemCoreClock / 100)) {
		while (1); /* Capture error */
   }

   if (!(SysTick->CTRL & SysTick_CTRL_CLKSOURCE_Msk)) {
        /* When external reference clock is used(CLKSOURCE in
		Systick Control and register bit 2 is set to 0), the
		SYSTICKCLKDIV must be a non-zero value and 2.5 times
		faster than the reference clock.
		When core clock, or system AHB clock, is used(CLKSOURCE
		in Systick Control and register bit 2 is set to 1), the
		SYSTICKCLKDIV has no effect to the SYSTICK frequency. See
		more on Systick clock and status register in Cortex-M3
		technical Reference Manual. */
		LPC_SYSCON->SYSTICKCLKDIV = 0x08;
	}


   GPIOInit();

   init_timers();


   // The LED on Xpresso
   /* Set port 0_7 to output */
   GPIOSetDir( LED_PORT, LED_BIT, 1 );
   GPIOSetValue( 0, 7, LEDvalue );

   // buttons
   buttons_init();


   // check for 'next' button
   if (GPIOGetValue(0,6)) {
	   led_digits_init();
	   //led_digits_set_blink(1);
	   led_red_set(1);
	   led_green_set(1);
	   led_green_set_blink(1);
   }
   else {
	   GPIOSetValue( 0, 7, 1 );
	   delay32Ms(0,500);
	   GPIOSetValue( 0, 7, 0 );
	   delay32Ms(0,500);
	   GPIOSetValue( 0, 7, 1 );
	   delay32Ms(0,500);
	   GPIOSetValue( 0, 7, 0 );
	   delay32Ms(0,500);
	   GPIOSetValue( 0, 7, 1 );
	   delay32Ms(0,500);
	   GPIOSetValue( 0, 7, 0 );
	   delay32Ms(0,500);
   }


   // BARCODE reader
   barcode_init();
   barcode_reset();

   // UART
   UARTInit(115200);
   // Enable the UART Interrupt
   NVIC_EnableIRQ(UART_IRQn);
   LPC_UART->IER = IER_RBR | IER_RLS;


   logger_setEnabled(1);
   logger_logStringln("/O:entering main loop..."); // send online message (means i'm online)

   /*
	GPIOSetValue( PORT1, 4, 0 );
	GPIOSetValue( PORT1, 11, 0 );
    //delay32Ms(0,2000);
    delay_clk(0,50000);
	GPIOSetValue( PORT1, 4, 1 );
	GPIOSetValue( PORT1, 11, 1 );*/

   /*
   UARTSendByte('1');


	//GPIOSetValue( PORT1, 4, 1 );
	GPIOSetValue( PORT1, 11, 1 );

	   UARTSendByte('2');
	delay32Ms(0,500);
	   UARTSendByte('3');
	GPIOSetValue( PORT1, 4, 0 );
	GPIOSetValue( PORT1, 11, 0 );
	delay32Ms(0,500);
	   UARTSendByte('4');

	GPIOSetValue( PORT1, 0, 0 );
	GPIOSetValue( PORT1, 1, 1 );
	GPIOSetValue( PORT1, 2, 1 );
	GPIOSetValue( PORT1, 3, 0 );
	delay32Ms(0,500);
	   UARTSendByte('5');



   //led_digits_set_value(11);
   delay32Ms(0,2000);
   UARTSendByte('6'); */

   //led_digits_disable();


   uint8_t counter = 0;
   for(counter = 0; counter < 100; counter++) {
	   led_digits_set_value(counter);
	   //delay_microseconds(1, 50000);
	   delay32Ms(0, 20);
   }

   for(counter = 0; counter < 10; counter++) {
	   led_digits_set_value(counter);
	   led_digits_set_value_by_chars('0' + counter, '0' + counter);
	   //delay_microseconds(1, 50000);
	   delay32Ms(0, 200);
   }

   delay32Ms(0,500);
   led_digits_set_value(0);
   led_red_set(0);

   while (1) {
      /* process logger */
      if (logger_dataAvailable() && UARTTxEmpty) {
         uint8_t iCounter;
         // fill transmit FIFO with 14 bytes
         for (iCounter = 0; iCounter < 14 && logger_dataAvailable(); iCounter++) {
            UARTSendByte(logger_read());
         }
       }

       barcode_process(msTicks);

       buttons_process(msTicks);

       led_digits_process(msTicks);

       if (barcode_is_data_available()) {
    	   uint8_t counter;
    	   // begin message
    	   logger_logString("/B:");
    	   for(counter = 0; counter < barcode_data_count; counter++) {
    		   logger_logByte(barcode_data[counter]);
    	   }
    	   logger_logCRLF();
    	   // this is important!
    	   barcode_reset();
       }

       if (barcode_is_error()) {
    	   logger_logString("/E:");
    	   logger_logNumber(barcode_get_error_code());
    	   logger_logCRLF();
    	   barcode_reset();
       }


       if (buttons_triggered(0)) {
    	   // on/off
    	   logger_logString("/S:");
    	   logger_logByte('0');
    	   logger_logByte('0');
    	   logger_logCRLF();
       }

       if (buttons_triggered(1)) {
    	   // on/off
    	   logger_logString("/S:");
    	   logger_logByte('1');
    	   logger_logByte('0');
    	   logger_logCRLF();
       }

       if (buttons_triggered(2)) {
    	   // on/off
    	   logger_logString("/S:");
    	   logger_logByte('2');
    	   logger_logByte('0');
    	   logger_logCRLF();
       }

       if (buttons_triggered(3)) {
    	   // on/off
           logger_logString("/S:");
    	   logger_logByte('3');
    	   logger_logByte('0');
    	   logger_logCRLF();
       }

       if (UARTCount != 0 && UARTBuffer[UARTCount-1] == 0x0A && UARTCount >= 3) {

    	   // command structure:
    	   // [0] = '/' => identifier
    	   // [1] = <command>
    	   // [2] = ':' => separator
    	   // [3] = <param1>
    	   // [4] = <param2>
    	   // etc.

    	   if (UARTBuffer[0] == '/' && UARTBuffer[2] == ':') {
    		   // heart beat message
    		   if (UARTBuffer[1] == 'H') {
    			   // do somegthing
    			   led_digits_set_blink(0);
    			   led_digits_set_blink_interval(500);
    			   led_green_set_blink(0);
    			   led_green_set(1);
    			   // send heart beat
    			   logger_logString("/H:0");
    			   logger_logCRLF();
    		   }
    		   // set led digits
    		   // e.g. /L:13 -> output 13
    		   if (UARTCount >= 5 && UARTBuffer[1] == 'L') {
    			   uint8_t digit1 = UARTBuffer[3];
    			   uint8_t digit0 = UARTBuffer[4];
    			   led_digits_set_value_by_chars(digit0, digit1);
    		   }
    		   // RPI is online
    		   // message '/O:'
    		   if (UARTBuffer[1] == 'O') {
    			   // do somegthing
    			   led_digits_set_blink(0);
    			   led_digits_set_blink_interval(500);
    		   }
    		   // shutdown message
    		   // message '/S:xx'
    		   // e.g. '/S:30' shutdown in 30 seconds
    		   if (UARTBuffer[1] == 'S') {
    			   // do somegthing
    		   }
    		   // message blink digits
    		   if (UARTCount >= 4 && UARTBuffer[1] == 'D') {
    			   // do somegthing
    			   if (UARTBuffer[3] == '1') {
    				   led_digits_set_blink(1);
    			   }
    			   else {
    				   led_digits_set_blink(0);
    			   }
    			   // e.g. '/D:15'
    			   if (UARTCount >= 5 && UARTBuffer[4] >= '0' && UARTBuffer[4] <= '9') {
    				   uint16_t interval = 100 * UARTBuffer[4];
    				   led_digits_set_blink_interval(interval);
    			   }
    		   }
    	   }
    	   UARTCount = 0;
       }

	}
}

/*********************************************************************************
**                            End Of File
*********************************************************************************/
