#include "throbber.h"

#include "led.h"

static const uint16_t kThrobPWMSteps = 180;         //Number of steps in PWM period
static const uint8_t kThrobMaxSpeed = 30;           //Number of steps in PWM period
static const uint16_t kThrobWaitTimeMax = 5000;     //Maximum wait time in ms between sequence
static const uint16_t kThrobEventTimeMax = 1800;    //Maximum time in seconds between events

enum ThrobStateEvent {
  kThrobStateEnter,
  kThrobStateUpdate
};

static uint32_t gThrobEventTime = 0;         //Timestamp of last event

void throbber_state_active(enum ThrobStateEvent event, uint32_t now);
void throbber_state_wait(enum ThrobStateEvent event, uint32_t now);

void (*gThrobStateFunc)(enum ThrobStateEvent, uint32_t) = &throbber_state_wait;

void throbber_init(void)
{
  led_init();
  gThrobStateFunc = &throbber_state_wait;
  (*gThrobStateFunc)(kThrobStateEnter, 0);
}

void throbber_set(uint32_t timestamp)
{
  gThrobEventTime = timestamp;
}

void throbber_update(uint32_t now)
{
  (*gThrobStateFunc)(kThrobStateUpdate, now);
}

void throbber_change_state(void (*state)(enum ThrobStateEvent, uint32_t), uint32_t now)
{
    gThrobStateFunc = state;
    (*gThrobStateFunc)(kThrobStateEnter, now);
}

uint16_t throbber_event_elapsed(uint32_t now)
{
  if (!gThrobEventTime)
    return kThrobEventTimeMax;
  uint32_t eventTime = (now - gThrobEventTime) / 1000;
  return eventTime < kThrobEventTimeMax ? eventTime : kThrobEventTimeMax;
}

uint8_t throbber_calculate_speed(uint32_t now)
{
  static uint8_t divisor = kThrobEventTimeMax / kThrobMaxSpeed;
  uint32_t speed = throbber_event_elapsed(now) / divisor;
  return speed ? speed : 1;
}

uint32_t throbber_calculate_delay(uint32_t now)
{
  static uint32_t divisor = kThrobEventTimeMax * 1000UL / kThrobWaitTimeMax;
  uint32_t offset = throbber_event_elapsed(now) * 1000UL / divisor;
  return now + kThrobWaitTimeMax - offset;
}

void throbber_state_active(enum ThrobStateEvent event, uint32_t now)
{
  static int16_t step = 0;      //Current PWM step position in period
  static int16_t value = 0;     //Current PWM brightness level; Max = kThrobPWMSteps
  static int8_t increment = 0;        //Number of steps to increment per update; sign value sets direction
  switch (event) {
  case kThrobStateEnter:
    step = 1;
    value = increment = throbber_calculate_speed(now);
    break;
  case kThrobStateUpdate:
    if (step < value)
      led_set();
    else
      led_clear();
    if (++step > kThrobPWMSteps) {
      step = 1;
      value += increment;
    }
    if (value > kThrobPWMSteps) {
      value = kThrobPWMSteps;
      increment *= -1;
    }
    if (value < 1) {
      led_clear();
      throbber_change_state(&throbber_state_wait, now);
      return;
    }
    break;
  }
}

void throbber_state_wait(enum ThrobStateEvent event, uint32_t now)
{
  static uint32_t exitTime = 0;
  switch (event) {
  case kThrobStateEnter:
    exitTime = throbber_calculate_delay(now);
    break;
  case kThrobStateUpdate:
    if (now > exitTime)
      throbber_change_state(&throbber_state_active, now);
    break;
  }
}
