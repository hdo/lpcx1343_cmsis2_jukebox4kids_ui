#ifndef __RDM630_H
#define __RDM630_H

#define RDM630_BUFFER_SIZE 16

void rdm630_init();
void rdm630_enable();
void rdm630_disable();
void rdm630_reset();
uint8_t rdm630_data_available();
void rdm630_read_data(uint8_t *buffer);
uint32_t rdm630_read_rfid_id();
void rdm630_process(uint32_t msticks);

#endif /* end __RDM630_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
