#ifndef __BUTTONS_H
#define __BUTTONS_H

#define BUTTON0 (1 << 1) // ON/OFF
#define BUTTON1 (1 << 2) // PREV
#define BUTTON2 (1 << 3) // PLAY/PAUSE
#define BUTTON3 (1 << 6) // NEXT

#define BUTTON_ON_OFF 0
#define BUTTON_PREV 1
#define BUTTON_PLAY_PAUSE 2
#define BUTTON_NEXT 3

void buttons_init(void);
uint32_t buttons_read_status(void);
void buttons_process(uint32_t msticks);
uint32_t buttons_triggered(uint8_t index);


#endif /* end __BUTTONS_H */
