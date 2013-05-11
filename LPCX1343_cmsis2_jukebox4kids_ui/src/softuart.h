#ifndef __SOFTUART_H
#define __SOFTUART_H

#define SOFTUART_BUFFER_SIZE 32

#define SOFTUART_PORT    PORT0
#define SOFTUART_RX_PIN  10
//#define SOFTUART_TX_PIN 9


void softuart_init();
void softuart_probe();
void softuart_reset();
void softuart_clear();
void softuart_start();
void softuart_enable();
void softuart_disable();
void softuart_process(uint32_t msticks);
void softuart_sample();

#endif /* end __SOFTUART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
