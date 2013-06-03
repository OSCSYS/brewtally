#ifndef BREWTALLY_LED_H_
#define BREWTALLY_LED_H_

#include <stdint.h>
#include <avr/io.h> 
 
//Function Declarations
void led_init(void);
void led_set(void);
void led_set_value(uint8_t value);
void led_clear(void);
void led_toggle(void);

#endif