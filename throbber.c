#include "throbber.h"

#include "led.h"

static const uint16_t kThrobPWMSteps = 180;         //Number of steps in PWM period
static const uint8_t kThrobMaxSpeed = 30;           //Number of steps in PWM period
static const uint16_t kThrobWaitTimeMax = 5000;     //Maximum wait time in ms between sequence
static const uint16_t kThrobEventTimeMax = 1800;    //Maximum time in seconds between events

enum ThrobStateEvent {
  kThrobStateEnter,
  kThrobStateUpdate,
  kThrobStateExit
};

static int16_t gThrobPwmStep = 0;         //Current PWM step position in period
static int16_t gThrobPwmValue = 0;         //Current PWM brightness level; Max = kThrobPWMSteps
static int8_t gThrobPwmInc = 0;        //Number of steps to increment per update; sign value sets direction
static uint32_t gThrobEventTime = 0;         //Timestamp of last event

void throbber_state_active_up(enum ThrobStateEvent event, uint32_t now);
void throbber_state_active_down(enum ThrobStateEvent event, uint32_t now);
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
    (*gThrobStateFunc)(kThrobStateExit, now);
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

void throbber_pwm_set(uint8_t pwmStep, uint8_t pwmValue)
{
  if (pwmStep < pwmValue)
    led_set();
  else
    led_clear();
}

void throbber_state_active_up(enum ThrobStateEvent event, uint32_t now)
{
  switch (event) {
  case kThrobStateEnter:
    gThrobPwmStep = 0;
    gThrobPwmInc = throbber_calculate_speed(now);
    gThrobPwmValue = gThrobPwmInc;
    break;
  case kThrobStateUpdate:
    if (gThrobPwmValue > kThrobPWMSteps) {
      throbber_change_state(&throbber_state_active_down, now);
      return;
    }
    if (++gThrobPwmStep > kThrobPWMSteps) {
      gThrobPwmStep = 0;
      gThrobPwmValue += gThrobPwmInc;
    }
    throbber_pwm_set(gThrobPwmStep, gThrobPwmValue);
    break;
  case kThrobStateExit:
    break;
  }
}

void throbber_state_active_down(enum ThrobStateEvent event, uint32_t now)
{
  switch (event) {
  case kThrobStateEnter:
    gThrobPwmStep = kThrobPWMSteps;
    gThrobPwmInc *= -1;
    gThrobPwmValue = kThrobPWMSteps;
    break;
  case kThrobStateUpdate:
    if (--gThrobPwmStep < 0) {
      gThrobPwmStep = kThrobPWMSteps;
      gThrobPwmValue += gThrobPwmInc;
    }
    if (gThrobPwmValue < 0) {
      throbber_change_state(&throbber_state_wait, now);
      return;
    }
    throbber_pwm_set(gThrobPwmStep, gThrobPwmValue);
    break;
  case kThrobStateExit:
    led_clear();
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
      throbber_change_state(&throbber_state_active_up, now);
    break;
  case kThrobStateExit:
    break;
  }
}
