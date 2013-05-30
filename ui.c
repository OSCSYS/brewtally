#include "ui.h"

#include <stdint.h>
#include <string.h>
#include <avr/io.h> 

#include "buttons.h"
#include "display.h"
#include "status.h"

//Sample size in ounces
static const uint8_t kSampleSize = 4;

static uint16_t gUiCount = 1;
static uint32_t gUiSampleTime = 0;

enum StateEvent {
  kStateEnter,
  kStateUpdate,
  kStateTimer,
  kStateExit
};

void ui_state_count(enum StateEvent);
void ui_state_ounces(enum StateEvent);
void ui_state_gallons(enum StateEvent);
void ui_state_elapsed(enum StateEvent);

void (*gUiStateFunc)(enum StateEvent) = &ui_state_count;

void ui_init(void)
{
  status_init();
  display_init();
  buttons_init();
  (*gUiStateFunc)(kStateEnter);
}

void ui_update()
{
  uint16_t lastCount = gUiCount;
  static uint32_t refreshTime = 0;
  uint32_t timestamp = millis();
  if (button_short(kButtonSample)) {
    ++gUiCount;
    gUiSampleTime = timestamp;
  }
  if (button_long(kButtonSample))
    --gUiCount;
  if (lastCount != gUiCount)
    (*gUiStateFunc)(kStateUpdate);
  if (timestamp - refreshTime > 999) {
    (*gUiStateFunc)(kStateTimer);
    refreshTime = timestamp;
  }
  if (button_short(kButtonSelect))
    (*gUiStateFunc)(kStateExit);
  if (button_long(kButtonSelect)) {
    //Not implemented
  }
    
}

void ui_state_count(enum StateEvent event)
{
  switch (event) {
    case kStateEnter:
      ui_state_count(kStateUpdate);
      display_write_string(1, " Cnt");
      break;
      
    case kStateUpdate:
      display_write_number(0, gUiCount, 0);
      break;
      
    case kStateTimer:
      break;
      
    case kStateExit:
      gUiStateFunc = &ui_state_ounces;
      (*gUiStateFunc)(kStateEnter);
      break;
  }
}

void ui_state_ounces(enum StateEvent event)
{
  switch (event) {
    case kStateEnter:
      ui_state_ounces(kStateUpdate);
      display_write_string(1, "  oZ");
      break;
      
    case kStateUpdate:
      display_write_number(0, gUiCount * kSampleSize, 0);
      break;
      
    case kStateTimer:
      break;
      
    case kStateExit:
      gUiStateFunc = &ui_state_gallons;
      (*gUiStateFunc)(kStateEnter);
      break;
  }
}

void ui_state_gallons(enum StateEvent event)
{
  switch (event) {
    case kStateEnter:
      ui_state_gallons(kStateUpdate);
      display_write_string(1, " GAL");
      break;
    case kStateUpdate:
      display_write_number(0, gUiCount * kSampleSize * 100 / 128, 2);
      break;
      
    case kStateTimer:
      break;
      
    case kStateExit:
      gUiStateFunc = &ui_state_elapsed;
      (*gUiStateFunc)(kStateEnter);
      break;
  }
}

void ui_state_elapsed(enum StateEvent event)
{
  switch (event) {
    case kStateEnter:
      ui_state_elapsed(kStateUpdate);
      display_write_string(1, "ELAP");
      break;

    case kStateUpdate:
    case kStateTimer:
      display_write_time(0, millis() - gUiSampleTime);
      break;
      
    case kStateExit:
      gUiStateFunc = &ui_state_count;
      (*gUiStateFunc)(kStateEnter);
      break;
  }
}
