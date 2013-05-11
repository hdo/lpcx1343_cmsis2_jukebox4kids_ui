#include "LPC13xx.h"
#include "gpio.h"
#include "barcode.h"
#include "logger.h"
#include "math_utils.h"
#include "timer16.h" // see LPC13xx_cmsis2_lib

volatile uint8_t barcode_data[BARCODE_BUFFER_SIZE];
volatile uint8_t barcode_current_bit = 0;
volatile uint8_t barcode_data_count = 0;
volatile uint32_t barcode_temp_data = 0;
volatile uint8_t barcode_last_byte = 0;
volatile uint8_t barcode_in_progress = 0;

/* hold msticks for time out handling */
volatile uint32_t barcode_last_signal = 0;
uint32_t barcode_msticks = 0;
uint8_t barcode_data_available = 0;
uint8_t barcode_error_occurred = 0;
uint8_t barcode_error_code = 0;

void barcode_init() {

	GPIOSetDir( BARCODE_PORT, BARCODE_CLK_PIN, 0 );
	GPIOSetDir( BARCODE_PORT, BARCODE_DATA_PIN, 0 );

	/* port0_1, single edge trigger on falling edge*/
	GPIOSetInterrupt( BARCODE_PORT, BARCODE_CLK_PIN, 0, 0, 0 );
}

void barcode_enable() {
	GPIOIntEnable( BARCODE_PORT, BARCODE_CLK_PIN );
}

void barcode_disable() {
	GPIOIntDisable( BARCODE_PORT, BARCODE_CLK_PIN );
}

void barcode_reset() {
	barcode_disable();
	barcode_current_bit = 0;
	barcode_data_count = 0;
	barcode_in_progress = 0;
	barcode_temp_data = 0;
	barcode_last_byte = 0;
	barcode_enable();
	barcode_data_available = 0;
}

void barcode_process(uint8_t msticks) {
	barcode_msticks = msticks;
	// check for 50ms time out
	// if after 50ms there is no new data then an error occurred
	// since the termination data hasn't been received
	if (barcode_in_progress && (math_calc_diff(barcode_msticks, barcode_last_signal) > 10)) {
		// error reading
		barcode_reset();

		barcode_error_occurred = 1;
		barcode_error_code = 1; // read time out
	}
}

uint8_t barcode_is_data_available() {
	return barcode_data_available;
}

uint8_t barcode_is_error() {
	if (barcode_error_occurred) {
		barcode_error_occurred = 0;
		return 1;
	}
	return 0;
}
uint8_t barcode_get_error_code() {
	return barcode_error_code;
}


void barcode_sample(void) {

	  // delay 5us for debouncing (using TIMER1)
	  delay_microseconds(1, 4);

	  // if clk is still low, then process valid signal
	  if (GPIOGetValue(BARCODE_PORT, BARCODE_CLK_PIN) == 0) {
		  barcode_in_progress = 1;
		  barcode_last_signal = barcode_msticks;

		  if (GPIOGetValue(BARCODE_PORT, BARCODE_DATA_PIN)) {
			  // least significant bit comes first
			  barcode_temp_data |= (1 << barcode_current_bit);
		  }

		  barcode_current_bit++;

		  /* barcode frame has 8 bits */
		  /* start bit, 8 data bits, parity bit, stop bit */
		  if (barcode_current_bit >= 11) {

			  /*
			  // check for start and stop bits
			  if (rawdata & 0x01 || (rawdata >> 10) ^ 0x01) {
				  // error start bit should be 0 and stop bit should be 1
				  logger_logStringln("barcode error: bit error");
			  }
			  */

			  // ignore start bit, parity bit and stop bit
			  uint8_t rawdata = (barcode_temp_data >> 1) & 0xFF;

			  // keyboard button released
			  // also see http://wiki.osdev.org/PS2_Keyboard
			  if (barcode_last_byte == 0xF0) {
				  // 0x5A ist enter (see http://wiki.osdev.org/PS2_Keyboard)
				  // if enter is received the data transfer is done
				  if (rawdata == 0x5A) {
					  // yeah data received;
					  //logger_logStringln("yeah!");

					  // set status
					  barcode_in_progress = 0;
					  barcode_data_available = 1;

					  // if data is available the interrupt is disabled
					  // so you need to read the data and re-enable the barcode
					  barcode_disable();
				  }
				  else {
					  /* actually only digits are processed the rest is ignored */
					  uint8_t data;
					  switch (rawdata) {
					  case 0x45 : data = '0'; break;
					  case 0x16 : data = '1'; break;
					  case 0x1E : data = '2'; break;
					  case 0x26 : data = '3'; break;
					  case 0x25 : data = '4'; break;
					  case 0x2E : data = '5'; break;
					  case 0x36 : data = '6'; break;
					  case 0x3D : data = '7'; break;
					  case 0x3E : data = '8'; break;
					  case 0x46 : data = '9'; break;
					  default: data = '?';
					  }
					  barcode_data[barcode_data_count++] = data;
					  //logger_logByte(data);

					  if (barcode_data_count >= BARCODE_BUFFER_SIZE) {
						  // buffer overflow
						  barcode_reset();
						  barcode_error_occurred = 1;
						  barcode_error_code = 2;
						  //logger_logStringln("barcode error: buffer overflow");
					  }
				  }
			  }

			  barcode_last_byte = rawdata;
			  barcode_current_bit = 0;
			  barcode_temp_data = 0;
		  }
	  }
}
