#ifndef __SOFTUART_H
#define __SOFTUART_H

#define SOFTUART_BUFFER_SIZE 32

void softuart_init();
void softuart_start_bit();
void softuart_probe(uint8_t value);
void softuart_stop_bit();
void softuart_reset();
void softuart_enable();
void softuart_disable();
void softuart_process(uint32_t msticks);

#endif /* end __SOFTUART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
