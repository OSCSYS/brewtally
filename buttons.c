#include "buttons.h"

#include <avr/interrupt.h>
#include <util/atomic.h> 

#include "display.h" //Provides millis()
#include "hwprofile.h"

static const uint16_t kButtonOKDuration = 50;
static const uint16_t kButtonCancelDuration = 1000;

//Global Encoder Variables
static volatile uint32_t gButtonStartTime[kButtonCount];
static volatile enum ButtonState gButtonState[kButtonCount];

void buttonISR(enum ButtonIndex button);

void buttons_init(void)
{
  for (uint8_t i = 0; i < kButtonCount; ++i) {
    gButtonStartTime[i] = 0;
    gButtonState[i] = kButtonStateIdle;
  }

  //Set pin directions and enable pull ups
  BUTTONS_DIR_REG &= ~(kButtonMasks[kButtonSelect] | kButtonMasks[kButtonSample] );
  BUTTONS_PULLUP_REG |= kButtonMasks[kButtonSelect] | kButtonMasks[kButtonSample];
  
  //Enable Buttons Interrupt
  MCUCR |= kButtonsInterruptSense;
  GIMSK |= kButtonsInterruptEn;
  
  //Enable global interrupts 
  sei();
}

uint8_t button_ok(enum ButtonIndex button)
{
  if(gButtonState[button] == kButtonStateOK) {
    gButtonState[button] = kButtonStateIdle;
    return 1;
  }
  return 0;
}

uint8_t button_cancel(enum ButtonIndex button)
{
  uint32_t timestamp = millis();
  uint32_t buttonStart;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { 
    buttonStart = gButtonStartTime[button];
  }
  cli();
  if((gButtonState[button] == kButtonStateCancel) || ((gButtonState[button] == kButtonStateClicked) && (buttonStart + kButtonCancelDuration < timestamp))) {
    gButtonState[button] = kButtonStateIdle;
    sei();
    return 1;
  }
  sei();
  return 0;
}

uint8_t button_raw(enum ButtonIndex button)
{
  return !(BUTTONS_INPUT_REG & kButtonMasks[button]);
}

ISR(BUTTON_SELECT_VECTOR) 
{
  cli();
  buttonISR(kButtonSelect);
  sei();
}

ISR(BUTTON_SAMPLE_VECTOR) 
{
  cli();
  buttonISR(kButtonSample);
  sei();
}

void buttonISR(enum ButtonIndex button) 
{
  switch (gButtonState[button]) {
    case kButtonStateIdle:
      //Button is pushed (ActiveLow)
      if (!(BUTTONS_INPUT_REG & kButtonMasks[button])) {    
        gButtonState[button] = kButtonStateClicked;
        gButtonStartTime[button] = millis();
      }
      break;
    case kButtonStateClicked:
      {
        //Assumes interrupt must be enter release
        uint16_t clickDuration = millis() - gButtonStartTime[button];
        if(clickDuration < kButtonOKDuration) {
          gButtonState[button] = kButtonStateIdle;
          break;
        }
        if (clickDuration < kButtonCancelDuration) {
          gButtonState[button] = kButtonStateOK;
          break;
        }
        gButtonState[button] = kButtonStateCancel;
      }
    default:
      //Events in OK/Cancel state ignored
      break;
  }
}