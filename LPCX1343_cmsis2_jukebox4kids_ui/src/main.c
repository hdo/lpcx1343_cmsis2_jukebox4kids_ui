#include "LPC13xx.h"
#include "gpio.h"
#include "barcode.h"
#include "uart.h"
#include "logger.h"
#include "buttons.h"
#include "led_digits.h"
#include "timer16.h"
#include "timer32.h"
#include "power_mgr.h"

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


int8_t get_char_to_int(uint8_t ch) {
	if (ch >= '0' && ch <= '9') {
		return ch - '0';
	}
	return -1;
}


void signal_boot_up() {
   led_digits_set_value(0);
   led_digits_set_blink(0);
   led_red_set(0);
   led_green_set(1);
   led_green_set_blink(1);
   led_red_set_blink(1);
}

void signal_shutting_down() {
   led_digits_set_blink(0);
   led_digits_enable();
   led_green_set(0);
   led_green_set_blink(0);
   led_red_set(1);
   led_red_set_blink(1);
}

void main_process_barcode() {
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
}

void main_process_buttons() {

    // ON/OFF
    if (buttons_triggered(0)) {
 	   // on/off
       if (power_mgr_get_player_status() == 0) {
    	   // power on
    	   power_mgr_set_player(1);
    	   power_mgr_set_amp(1);
    	   signal_boot_up();
       }
       else {
     	   logger_logString("/S:");
     	   logger_logByte('0');
     	   logger_logByte('0');
     	   logger_logCRLF();
       }
    }

    // PREV
    if (buttons_triggered(1)) {
 	   // on/off
 	   logger_logString("/S:");
 	   logger_logByte('1');
 	   logger_logByte('0');
 	   logger_logCRLF();
    }

    // PLAY/PAUSE
    if (buttons_triggered(2)) {
 	   // on/off
 	   logger_logString("/S:");
 	   logger_logByte('2');
 	   logger_logByte('0');
 	   logger_logCRLF();
    }

    // NEXT
    if (buttons_triggered(3)) {
 	   // on/off
        logger_logString("/S:");
 	   logger_logByte('3');
 	   logger_logByte('0');
 	   logger_logCRLF();
    }
}

void main_process_uart() {
    if (UARTCount > 0 && UARTBuffer[UARTCount-1] == 0x0A) {
 	   // command structure:
 	   // [0] = '/' => identifier
 	   // [1] = <command>
 	   // [2] = ':' => separator
 	   // [3] = <param1>
 	   // [4] = <param2>
 	   // etc.
 	   // [n] = 0x0A

 	   if (UARTCount > 3 && UARTBuffer[0] == '/' && UARTBuffer[2] == ':') {
 		   uint8_t command = UARTBuffer[1];

 		   // heart beat message from player
 		   if (command == 'H') {
 			   led_digits_set_blink(0);
 			   led_digits_set_blink_interval(500);
 			   led_green_set_blink(0);
 			   led_red_set_blink(0);
 			   led_red_set(0);
 			   led_green_set(1);
 			   // send heart beat
 			   logger_logString("/H:0");
 			   logger_logCRLF();
 		   }

 		   // set led digits
 		   // e.g. /L:13 -> output 13
 		   if (command == 'L' && UARTCount > 5) {
 			   uint8_t digit1 = UARTBuffer[3];
 			   uint8_t digit0 = UARTBuffer[4];
 			   led_digits_set_value_by_chars(digit0, digit1);
 		   }

 		   // set led
 		   // "/l:R0" -> led red off
 		   // "/l:R1" -> led red on
 		   // "/l:R2" -> led red blink off
 		   // "/l:R3" -> led red blink on
 		   // "/l:G0" -> led green off
 		   // "/l:G1" -> led green on
 		   // "/l:G2" -> led green blink off
 		   // "/l:G3" -> led green blink on
 		   if (command == 'l' && UARTCount > 5) {
 			   uint8_t param1 = UARTBuffer[3];
 			   uint8_t param2 = UARTBuffer[4];
 			   if (param1 == 'R') {
 				   switch (param2) {
 				   case 0 : led_red_set(0); break;
 				   case 1 : led_red_set(1); break;
 				   case 2 : led_red_set_blink(0); break;
 				   case 3 : led_red_set_blink(1); break;
 				   }
 			   }
 			   if (param1 == 'G') {
 				   switch (param2) {
 				   case 0 : led_green_set(0); break;
 				   case 1 : led_green_set(1); break;
 				   case 2 : led_green_set_blink(0); break;
 				   case 3 : led_green_set_blink(1); break;
 				   }
 			   }
 		   }


 		   // shutdown message
 		   // message '/S:xx'
 		   // e.g. '/S:30' shutdown in 30 seconds
 		   if (command == 'S' && UARTCount > 5) {
 			   // do somegthing
 			   int8_t param1 = get_char_to_int(UARTBuffer[3]);
 			   int8_t param2 = get_char_to_int(UARTBuffer[4]);
 			   if (param1 > -1 && param2 > -1) {
 				   signal_shutting_down();
 				   int8_t seconds = param1*10 + param2;
 				   power_mgr_shutdown_player(seconds);
 			   }
 		   }


 		   // message blink digits
 		   if (command == 'D' && UARTCount > 4) {
 			   uint8_t param1 = UARTBuffer[3];
 			   // do somegthing
 			   if (param1 == '1') {
 				   led_digits_set_blink(1);
 			   }
 			   else {
 				   led_digits_set_blink(0);
 			   }
 			   /*
 			   // e.g. '/D:15'
 			   if (UARTCount >= 5 && UARTBuffer[4] >= '0' && UARTBuffer[4] <= '9') {
 				   uint16_t interval = 100 * UARTBuffer[4];
 				   led_digits_set_blink_interval(interval);
 			   }
 			   */
 		   }
 	   }
 	   UARTCount = 0;
    }
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
   if (GPIOGetValue(0, 6)) {
	   led_digits_init();
	   //led_digits_set_blink(1);
	   led_red_set(1);
	   led_green_set(1);
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

   uint8_t counter = 0;
   for(counter = 0; counter < 100; counter++) {
	   led_digits_set_value(counter);
	   delay32Ms(0, 20);
   }

   for(counter = 0; counter < 10; counter++) {
	   led_digits_set_value_by_chars('0' + counter, '0' + counter);
	   delay32Ms(0, 200);
   }
   delay32Ms(0, 200);
   //led_digits_disable();

   signal_boot_up();

   int8_t last_remaining_seconds = -1;
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

	   if (power_mgr_is_shutting_down()) {
		   if (power_mgr_get_remaining_player_seconds() != last_remaining_seconds) {
			   last_remaining_seconds = power_mgr_get_remaining_player_seconds();
			   led_digits_set_value(last_remaining_seconds);
		   }
		   if (last_remaining_seconds == 0) {
			   led_green_set(0);
			   led_red_set(1);
			   led_digits_disable();
		   }
		   continue;
	   }

       main_process_barcode();

       main_process_buttons();

       main_process_uart();

   }
}

/*********************************************************************************
**                            End Of File
*********************************************************************************/
