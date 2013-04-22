#ifndef __BUTTONS_H
#define __BUTTONS_H

#define BUTTON0 (1 << 1)
#define BUTTON1 (1 << 2)
#define BUTTON2 (1 << 3)
#define BUTTON3 (1 << 6)


void buttons_init(void);
uint32_t buttons_read_status(void);
void buttons_process(uint32_t msticks);
uint32_t buttons_triggered(uint8_t index);


#endif /* end __BUTTONS_H */
