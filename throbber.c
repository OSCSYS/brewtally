#include "throbber.h"


#include "led.h"

static const uint8_t kThrobs = 4;                     //Number of toggles (flashes x 2) per cycle
static const uint16_t kThrobPulseTimeMax = 1000;       //Amount of On time in ms per throb
static const uint16_t kThrobPulseTimeRange = 875;     //Set range equal to max for a range of 0-max or less than range to force a minimum value
static const uint16_t kThrobWaitTimeMax = 5000;       //Maximum wait time in ms between sequence
static const uint16_t kThrobWaitTimeRange = 4875;     //Set range equal to max for a range of 0-max or less than range to force a minimum value
static const uint16_t kThrobEventTimeMax = 1800;      //Maximum time in s between throbs

enum ThrobStateEvent {
  kThrobStateEnter,
  kThrobStateUpdate
};

static uint8_t gThrobCount = 0;             //Current throb count position in series
static uint32_t gThrobTimestamp = 0;         //Timestamp of last event
static uint32_t gThrobStepTime = 0;          //Timestamp marking the start of the current active step

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
  gThrobTimestamp = timestamp;
}

void throbber_update(uint32_t now)
{
  (*gThrobStateFunc)(kThrobStateUpdate, now);
}

void throbber_change_state(void (*state)(enum ThrobStateEvent, uint32_t), uint32_t now)
{
    gThrobStateFunc = state;
    gThrobStepTime = now;
    (*gThrobStateFunc)(kThrobStateEnter, now);
}

uint8_t throbber_is_step_complete(uint32_t now, uint16_t max, uint16_t range)
{
  uint32_t stepMs = now - gThrobStepTime;
  uint32_t eventS = (now - gThrobTimestamp) / 1000;
  if (eventS > kThrobEventTimeMax) eventS = kThrobEventTimeMax;
  if (!gThrobTimestamp) eventS = kThrobEventTimeMax; //Boot Thirsty
  return ((max - (eventS * range / kThrobEventTimeMax)) > stepMs) ? 0 : 1;
}

void throbber_state_active(enum ThrobStateEvent event, uint32_t now)
{
  switch (event) {
  case kThrobStateEnter:
    led_toggle();
    if (++gThrobCount == kThrobs)
      throbber_change_state(&throbber_state_wait, now);
    break;
  case kThrobStateUpdate:
    if (throbber_is_step_complete(now, kThrobPulseTimeMax, kThrobPulseTimeRange))
      throbber_change_state(&throbber_state_active, now);
    break;
  }
}

void throbber_state_wait(enum ThrobStateEvent event, uint32_t now)
{
  switch (event) {
  case kThrobStateEnter:
    gThrobCount = 0;
    break;
  case kThrobStateUpdate:
    if (throbber_is_step_complete(now, kThrobWaitTimeMax, kThrobWaitTimeRange))
      throbber_change_state(&throbber_state_active, now);
    break;
  }
}
