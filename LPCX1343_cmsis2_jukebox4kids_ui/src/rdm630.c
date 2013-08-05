#include "LPC13xx.h"
#include "gpio.h"
#include "rdm630.h"
#include "softuart.h"
#include "logger.h"
#include "math_utils.h"

uint32_t rdm630_last_received_msticks = 0;
uint8_t rdm630_old_buffer[RDM630_BUFFER_SIZE];
uint8_t rdm630_current_buffer[RDM630_BUFFER_SIZE];
uint8_t rdm630_is_data_available_flag = 0;

trigger_config_t rdm630_trigger_config = {.start_trigger = 0x02, .stop_trigger=0x3, .start_trigger_enabled = 1, .stop_trigger_enabled = 1};


void rdm630_init() {
	softuart_init();
    softuart_set_trigger_config(&rdm630_trigger_config);
    uint8_t i;
    for(i = 0; i < RDM630_BUFFER_SIZE; i++) {
    	rdm630_old_buffer[i] = 0;
    	rdm630_current_buffer[i] = 0;
    }
}

void update_old_buffer() {
	uint8_t i;
	for(i = 0; i < RDM630_BUFFER_SIZE; i++) {
		rdm630_old_buffer[i] = rdm630_current_buffer[i];
		// init current_buffer with 0 values
		rdm630_current_buffer[i] = 0;
	}
}

uint8_t is_buffer_equal() {
	uint8_t i;
	for(i = 0; i < RDM630_BUFFER_SIZE; i++) {
		if (rdm630_old_buffer[i] != rdm630_current_buffer[i]) {
			return 0;
		}
	}
	return 1;
}

void rdm630_enable() {
	softuart_enable();
}

void rdm630_disable() {
	softuart_disable();
}

void rdm630_reset() {
	rdm630_is_data_available_flag = 0;
	softuart_reset();
}

uint8_t rdm630_data_available() {
	return rdm630_is_data_available_flag;
}

void rdm630_read_data(uint8_t *buffer) {
}

uint8_t char_to_hex(uint8_t cdata) {
	switch(cdata) {
	case '0' : return 0x00;
	case '1' : return 0x01;
	case '2' : return 0x02;
	case '3' : return 0x03;
	case '4' : return 0x04;
	case '5' : return 0x05;
	case '6' : return 0x06;
	case '7' : return 0x07;
	case '8' : return 0x08;
	case '9' : return 0x09;
	case 'A' : return 0x0A;
	case 'B' : return 0x0B;
	case 'C' : return 0x0C;
	case 'D' : return 0x0D;
	case 'E' : return 0x0E;
	case 'F' : return 0x0F;
	case 'a' : return 0x0A;
	case 'b' : return 0x0B;
	case 'c' : return 0x0C;
	case 'd' : return 0x0D;
	case 'e' : return 0x0E;
	case 'f' : return 0x0F;
	}
	return 0x00;
}

uint32_t rdm630_read_rfid_id() {
	uint8_t i;
	uint32_t rfid = 0;
	for(i = 3; i < 11; i++) {
		uint8_t hdata = char_to_hex(rdm630_current_buffer[i]);
		rfid |= hdata;
		if (i < 10) {
			rfid = rfid << 4;
		}
		//logger_logByte(rdm630_current_buffer[i]);
	}
	return rfid;
}

void rdm630_process(uint32_t msticks) {
	if (softuart_data_available() && softuart_done_receiving()) {
		update_old_buffer();
		uint8_t *incoming = rdm630_current_buffer;
		while(softuart_data_available()) {
			*incoming++ = softuart_read_byte();
		}
		if (!is_buffer_equal()) {
			rdm630_is_data_available_flag = 1;
		}
		else {
			// some cards are sending their IDs continuously - we don't want to read those IDs again
			// the user have to wait at least 1500 ms
			if (math_calc_diff(msticks, rdm630_last_received_msticks) > 150) {
				rdm630_is_data_available_flag = 1;
			}
			else {
				// WARNING: don't forget this!
				// reset for reading next rfid id
				rdm630_reset();
			}
		}
		rdm630_last_received_msticks = msticks;
	}

}
