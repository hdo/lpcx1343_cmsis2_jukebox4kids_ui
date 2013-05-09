#include "LPC13xx.h"
#include "gpio.h"
#include "softuart.h"
#include "logger.h"
#include "math_utils.h"
#include "queue.h"

uint8_t softuart_buffer_data[SOFTUART_BUFFER_SIZE];
ringbuffer_t softuart_rbuffer = {.buffer=softuart_buffer_data, .head=0, .tail=0, .count=0, .size=SOFTUART_BUFFER_SIZE};



void softuart_init() {

}

void softuart_start_bit() {

}

void softuart_probe() {

}

void softuart_stop_bit() {

}

void softuart_reset() {

}

void softuart_enable() {

}

void softuart_disable() {

}

void softuart_process(uint32_t msticks) {

}
