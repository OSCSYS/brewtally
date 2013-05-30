#include "status.h"

//Status output registers
#define STATUS_OUTPUT_REG   PORTA

//Status direction registers
#define STATUS_DIR_REG      DDRA

//Status pin bitmask
static const uint8_t kStatusPinMask  = 0x02;


void status_init(void)
{
  //Set pin directions
  STATUS_DIR_REG |= kStatusPinMask;
}

void status_set(void)
{
  STATUS_OUTPUT_REG |= kStatusPinMask;
}

void status_clear(void)
{
  STATUS_OUTPUT_REG &= ~kStatusPinMask;
}

void status_toggle(void)
{
  STATUS_OUTPUT_REG ^= kStatusPinMask;
}