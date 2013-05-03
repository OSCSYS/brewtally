#include "buttons.h"

#include <avr/interrupt.h>
#include <util/atomic.h> 

#include "hwprofile.h"

#define REPEAT_MASK (kButtonSelect | kButtonSample)  //Enable repeat logic
#define REPEAT_START 100 //After 1s 
#define REPEAT_NEXT 50  //Every 500ms

//Global Button Variables
static volatile uint8_t gButtonState;
static volatile uint8_t gButtonPress;
static volatile uint8_t gButtonRepeat; 

ISR(BUTTON_TIMER_VECTOR)
{
  static uint8_t count0, count1, repeat;
  uint8_t i;
 
  i = gButtonState ^ ~BUTTONS_INPUT_REG;             //Button changed?
  count0 = ~(count0 & i);                            //Reset or count count0
  count1 = count0 ^ (count1 & i);                    //Reset or count count1
  i &= count0 & count1;                              //Count until roll over ?
  gButtonState ^= i;                                 //Then toggle debounced state
  gButtonPress |= gButtonState & i;                  //0->1: Button press detect
 
  if( (gButtonState & REPEAT_MASK) == 0 )            //Check repeat function
     repeat = REPEAT_START;                          //Start delay
  if( --repeat == 0 ){
    repeat = REPEAT_NEXT;                            //Repeat delay
    gButtonRepeat |= gButtonState & REPEAT_MASK;
  }
}

void buttons_init(void)
{
  //Set pin directions and enable pull ups
  BUTTONS_DIR_REG &= ~(kButtonSelect | kButtonSample);
  BUTTONS_PULLUP_REG |= kButtonSelect | kButtonSample;
  
  BUTTON_TIMER_MODE_REG |= kButtonTimerMode;                    //Configure timer for CTC mode 
  BUTTON_TIMER_INTERRUPT_MASK_REG |= kButtonTimerInterruptMask; //Enable timer interrupt
  sei();                                                        //Enable global interrupts 
  BUTTON_TIMER_COMPARE_VALUE_REG = kButtonTimerCompareValue;    //Set compare value for a compare rate of 1kHz 
  BUTTON_TIMER_PRESCALER_REG |= kButtonTimerPrescaler;           //Set timer prescaler
}

uint8_t button_press(uint8_t button)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    button &= gButtonPress;
    gButtonPress ^= button;
  }
  return button;
}

uint8_t button_repeat(uint8_t button)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    button &= gButtonRepeat;
    gButtonRepeat ^= button;
  }
  return button;
}

uint8_t button_state(uint8_t button)
{
  button &= gButtonState;
  return button;
}

uint8_t button_short (uint8_t button)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    button = button_press(~gButtonState & button);
  }
  return button;
}
 
uint8_t button_long (uint8_t button)
{
  return button_press(button_repeat(button));
}

uint8_t button_raw (uint8_t button)
{
  return (button & (~BUTTONS_INPUT_REG));
}