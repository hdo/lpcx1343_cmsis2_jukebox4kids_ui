#ifndef __BARCODE_H
#define __BARCODE_H

#define BARCODE_PORT     PORT0
#define BARCODE_CLK_PIN  8
#define BARCODE_DATA_PIN 9


#define BARCODE_BUFFER_SIZE 32

void barcode_init();
void barcode_enable();
void barcode_disable();
void barcode_reset();
void barcode_process(uint8_t msticks);
uint8_t barcode_read();
uint8_t barcode_is_data_available();
uint8_t barcode_is_error();
uint8_t barcode_get_error_code();
void barcode_probe();

#endif /* end __BARCODE_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
