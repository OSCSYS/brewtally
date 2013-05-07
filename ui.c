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

enum StateEvent {
  kStateEnter,
  kStateUpdate,
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
  if (button_short(kButtonSample))
    ++gUiCount;
  if (button_long(kButtonSample))
    --gUiCount;
  if (lastCount != gUiCount)
    (*gUiStateFunc)(kStateUpdate);
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
      display_write_string(1, " Cnt");
      
    case kStateUpdate:
      display_write_number(0, gUiCount, 0);
      if (event == kStateEnter) display_frame_focus(1);
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
      display_write_string(1, "  oZ");
      
    case kStateUpdate:
      display_write_number(0, gUiCount * kSampleSize, 0);
      if (event == kStateEnter) display_frame_focus(1);
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
      display_write_string(1, " GAL");
      
    case kStateUpdate:
      display_write_number(0, gUiCount * kSampleSize * 100 / 128, 2);
      if (event == kStateEnter) display_frame_focus(1);
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
      display_write_string(1, "ELAP");
      
    case kStateUpdate:
      display_write_number(0, gUiCount, 0);
      if (event == kStateEnter) display_frame_focus(1);
      break;
      
    case kStateExit:
      gUiStateFunc = &ui_state_count;
      (*gUiStateFunc)(kStateEnter);
      break;
  }
}
