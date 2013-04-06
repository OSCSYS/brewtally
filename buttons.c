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

void button_doClick(enum ButtonIndex button);

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
  switch (gButtonState[kButtonSelect]) {
    case kButtonStateIdle:
      //Button is pushed (ActiveLow)
      if (!(BUTTONS_INPUT_REG & kButtonMasks[kButtonSelect])) {    
        gButtonState[kButtonSelect] = kButtonStateClicked;
        gButtonStartTime[kButtonSelect] = millis();
      }
      break;
    case kButtonStateClicked:
      {
        //Assumes interrupt must be enter release
        uint16_t clickDuration = millis() - gButtonStartTime[kButtonSelect];
        if(clickDuration < kButtonOKDuration) {
          gButtonState[kButtonSelect] = kButtonStateIdle;
          break;
        }
        if (clickDuration < kButtonCancelDuration) {
          gButtonState[kButtonSelect] = kButtonStateOK;
          break;
        }
        gButtonState[kButtonSelect] = kButtonStateCancel;
      }
    default:
      //Events in OK/Cancel state ignored
      break;
  }
  sei();
}

ISR(BUTTON_SAMPLE_VECTOR) 
{
  cli();
  switch (gButtonState[kButtonSample]) {
    case kButtonStateIdle:
      //Button is pushed (ActiveLow)
      if (!(BUTTONS_INPUT_REG & kButtonMasks[kButtonSample])) {    
        gButtonState[kButtonSample] = kButtonStateClicked;
        gButtonStartTime[kButtonSample] = millis();
      }
      break;
    case kButtonStateClicked:
      {
        //Assumes interrupt must be enter release
        uint16_t clickDuration = millis() - gButtonStartTime[kButtonSample];
        if(clickDuration < kButtonOKDuration) {
          gButtonState[kButtonSample] = kButtonStateIdle;
          break;
        }
        if (clickDuration < kButtonCancelDuration) {
          gButtonState[kButtonSample] = kButtonStateOK;
          break;
        }
        gButtonState[kButtonSample] = kButtonStateCancel;
      }
    default:
      //Events in OK/Cancel state ignored
      break;
  }
  sei();
}
