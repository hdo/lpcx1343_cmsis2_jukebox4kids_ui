#ifndef __LED_DIGITS_H
#define __LED_DIGITS_H

#define LED_DIGITS_PORT PORT1
#define LED_DGITIS_PORT_BL PORT0
#define LED_DIGITS_PIN_A 0
#define LED_DIGITS_PIN_B 1
#define LED_DIGITS_PIN_C 2
#define LED_DIGITS_PIN_D 3
#define LED_DIGITS_PIN_L0 4
#define LED_DIGITS_PIN_L1 11
#define LED_DIGITS_PIN_BL 11

#define LED_RED_PORT PORT0
#define LED_RED_PIN 7
#define LED_GREEN_PORT PORT3
#define LED_GREEN_PIN 2

void led_digits_init();
void led_digits_process(uint32_t msticks);
void led_digits_enable();
void led_digits_disable();
void led_digits_set_value(uint8_t value);
void led_digits_set_value_by_chars(uint8_t ch0, uint8_t ch1);
void led_digits_set_blink(uint8_t enabled);
void led_digits_set_blink_interval(uint16_t interval);
void led_red_set(uint8_t enabled);
void led_green_set(uint8_t enbaled);
void led_red_set_blink(uint8_t enabled);
void led_green_set_blink(uint8_t enabled);



#endif /* end __LED_DIGITS_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
