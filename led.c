#include "led.h"

#define LED_OUTPUT_REG   PORTA            //Status output registers
#define LED_DIR_REG      DDRA             //Status direction registers

static const uint8_t kLedPinMask  = 0x02; //Status pin bitmask

void led_init(void)
{
  LED_DIR_REG |= kLedPinMask;       //Set pin directions
  LED_OUTPUT_REG &= ~kLedPinMask;   //Force off on init
}

void led_set(void)
{
  LED_OUTPUT_REG |= kLedPinMask;
}

void led_set_value(uint8_t value)
{
  if (value)
    led_set();
  else
    led_clear();
}

void led_clear(void)
{
  LED_OUTPUT_REG &= ~kLedPinMask;
}

void led_toggle(void)
{
  LED_OUTPUT_REG ^= kLedPinMask;
}