#ifndef __SOFTUART_H
#define __SOFTUART_H

#define SOFTUART_BUFFER_SIZE 32

#define SOFTUART_PORT    PORT0
#define SOFTUART_RX_PIN  10
//#define SOFTUART_RX_PIN  6
//#define SOFTUART_TX_PIN 9

typedef struct trigger_config {
	uint8_t start_trigger;
    uint8_t stop_trigger;
    uint8_t start_trigger_enabled;
    uint8_t stop_trigger_enabled;
} trigger_config_t;


void softuart_init();
void softuart_reset();
void softuart_start_sampling();
void softuart_enable();
void softuart_disable();
void softuart_set_trigger_config(trigger_config_t *triggerconf);
uint8_t softuart_data_available();
uint8_t softuart_done_receiving();
uint8_t softuart_data_count();
uint8_t softuart_read_byte();
void softuart_process(uint32_t msticks);
void softuart_sample();

#endif /* end __SOFTUART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
