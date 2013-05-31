#include "ui.h"

#include <stdint.h>
#include <string.h>
#include <avr/io.h> 

#include "buttons.h"
#include "display.h"
#include "throbber.h"

//Sample size in ounces
static const uint8_t kSampleSize = 4;

static uint16_t gUiCount = 0;
static uint32_t gUiSampleTime = 0;

enum UIStateEvent {
  kUIStateEnter,
  kUIStateUpdate,
  kUIStateTimer,
  kUIStateExit
};

void ui_state_count(enum UIStateEvent);
void ui_state_ounces(enum UIStateEvent);
void ui_state_gallons(enum UIStateEvent);
void ui_state_elapsed(enum UIStateEvent);

void (*gUiStateFunc)(enum UIStateEvent) = &ui_state_count;

void ui_init(void)
{
  throbber_init();
  display_init();
  buttons_init();
  (*gUiStateFunc)(kUIStateEnter);
}

void ui_update()
{
  uint16_t lastCount = gUiCount;
  static uint32_t refreshTime = 0;
  uint32_t timestamp = millis();
  if (button_short(kButtonSample)) {
    ++gUiCount;
    gUiSampleTime = timestamp;
    throbber_set(gUiSampleTime);
  }
  if (button_long(kButtonSample))
    --gUiCount;
  if (lastCount != gUiCount)
    (*gUiStateFunc)(kUIStateUpdate);

  if (timestamp - refreshTime > 999) {
    (*gUiStateFunc)(kUIStateTimer);
    refreshTime = timestamp;
  }
  throbber_update(timestamp);
  if (button_short(kButtonSelect))
    (*gUiStateFunc)(kUIStateExit);
  if (button_long(kButtonSelect)) {
    //Not implemented
  }
}

void ui_state_count(enum UIStateEvent event)
{
  switch (event) {
    case kUIStateEnter:
      ui_state_count(kUIStateUpdate);
      display_write_string(1, " Cnt");
      break;
      
    case kUIStateUpdate:
      display_write_number(0, gUiCount, 0);
      break;
      
    case kUIStateTimer:
      break;
      
    case kUIStateExit:
      gUiStateFunc = &ui_state_ounces;
      (*gUiStateFunc)(kUIStateEnter);
      break;
  }
}

void ui_state_ounces(enum UIStateEvent event)
{
  switch (event) {
    case kUIStateEnter:
      ui_state_ounces(kUIStateUpdate);
      display_write_string(1, "  oZ");
      break;
      
    case kUIStateUpdate:
      display_write_number(0, gUiCount * kSampleSize, 0);
      break;
      
    case kUIStateTimer:
      break;
      
    case kUIStateExit:
      gUiStateFunc = &ui_state_gallons;
      (*gUiStateFunc)(kUIStateEnter);
      break;
  }
}

void ui_state_gallons(enum UIStateEvent event)
{
  switch (event) {
    case kUIStateEnter:
      ui_state_gallons(kUIStateUpdate);
      display_write_string(1, " GAL");
      break;
    case kUIStateUpdate:
      display_write_number(0, gUiCount * kSampleSize * 100 / 128, 2);
      break;
      
    case kUIStateTimer:
      break;
      
    case kUIStateExit:
      gUiStateFunc = &ui_state_elapsed;
      (*gUiStateFunc)(kUIStateEnter);
      break;
  }
}

void ui_state_elapsed(enum UIStateEvent event)
{
  switch (event) {
    case kUIStateEnter:
      ui_state_elapsed(kUIStateUpdate);
      display_write_string(1, "ELAP");
      break;

    case kUIStateUpdate:
    case kUIStateTimer:
      display_write_time(0, millis() - gUiSampleTime);
      break;
      
    case kUIStateExit:
      gUiStateFunc = &ui_state_count;
      (*gUiStateFunc)(kUIStateEnter);
      break;
  }
}
