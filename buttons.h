#ifndef BREWTALLY_BUTTONS_H_
#define BREWTALLY_BUTTONS_H_

#include <stdint.h>
#include <avr/io.h> 

enum ButtonState {
  kButtonStateIdle,
  kButtonStateClicked,
  kButtonStateOK,
  kButtonStateCancel
};

enum ButtonIndex {
  kButtonSelect,
  kButtonSample,
  kButtonCount
};
  
//Function declarations
void buttons_init(void);
uint8_t button_ok(enum ButtonIndex);
uint8_t button_cancel(enum ButtonIndex);
uint8_t button_raw(enum ButtonIndex);

#endif