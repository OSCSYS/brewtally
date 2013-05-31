#include "led.h"

//Status output registers
#define LED_OUTPUT_REG   PORTA

//Status direction registers
#define LED_DIR_REG      DDRA

//Status pin bitmask
static const uint8_t kLedPinMask  = 0x02;


void led_init(void)
{
  //Set pin directions
  LED_DIR_REG |= kLedPinMask;
}

void led_set(void)
{
  LED_OUTPUT_REG |= kLedPinMask;
}

void led_clear(void)
{
  LED_OUTPUT_REG &= ~kLedPinMask;
}

void led_toggle(void)
{
  LED_OUTPUT_REG ^= kLedPinMask;
}