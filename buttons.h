#ifndef BREWTALLY_BUTTONS_H_
#define BREWTALLY_BUTTONS_H_

#include <stdint.h>
#include <avr/io.h> 

enum ButtonMask {
  kButtonSelect = _BV(2),
  kButtonSample = _BV(3)
};

//Function declarations
void buttons_init(void);

//Check if a button has been pressed.
//Each pressed button is reported only once
uint8_t button_press(uint8_t button);

//Check if a button has been pressed long enough such that the
//button repeat functionality kicks in. After a small setup delay
//the button is reported being pressed in subsequent calls
//to this function. This simulates the user repeatedly
//pressing and releasing the button.
uint8_t button_repeat(uint8_t button);

//Check if a key is pressed right now
uint8_t button_state(uint8_t button);

uint8_t button_short(uint8_t button);

uint8_t button_long(uint8_t button);

uint8_t button_raw(uint8_t button);
#endif