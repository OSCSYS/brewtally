#include "ui.h"

#include <stdint.h>
#include <string.h>
#include <avr/io.h> 

#include "buttons.h"
#include "display.h"
#include "throbber.h"

static const uint32_t kUiSleepTimeout = 7200000UL;

static uint16_t gUiCount = 0;
static uint32_t gUiSampleTime = 0;
static uint32_t gUiSleepTime = 0;
static uint8_t gUiSampleSize = 4; //Sample size in ounces

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
void ui_configure_menu(void);
void ui_change_state(void (*state)(enum UIStateEvent));
void ui_update_sleep_timer(uint32_t timeout);
uint8_t ui_is_sleepy(uint32_t now);

void (*gUiStateFunc)(enum UIStateEvent) = 0;

void ui_init(void)
{
  throbber_init();
  display_init();
  uint8_t brightness = 0;
  display_set_brightness(brightness);
  buttons_init();
}

void ui_update()
{
  if(!gUiStateFunc) {
    //Sleep Logic
    if (button_press(kButtonSelect | kButtonSample))
      ui_change_state(&ui_state_count);
    return;
  }
  uint32_t timestamp = millis();
  uint16_t lastCount = gUiCount;
  static uint32_t refreshTime = 0;
  if (timestamp > gUiSleepTime) {
    throbber_init();
    display_clear();
    gUiStateFunc = 0;
    return;
  }
  if (button_short(kButtonSample)) {
    ++gUiCount;
    gUiSampleTime = timestamp;
    throbber_set(gUiSampleTime);
    gUiSleepTime = timestamp + kUiSleepTimeout;
  }
  if (button_long(kButtonSample))
    --gUiCount;
  if (button_short(kButtonSelect))
    (*gUiStateFunc)(kUIStateExit);
  if (button_long(kButtonSelect))
    ui_configure_menu();
  if (lastCount != gUiCount)
    (*gUiStateFunc)(kUIStateUpdate);
  if ((timestamp - refreshTime) / 1000) {
    (*gUiStateFunc)(kUIStateTimer);
    refreshTime = timestamp;
  }
  throbber_update(timestamp);
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
      ui_change_state(&ui_state_ounces);
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
      display_write_number(0, gUiCount * gUiSampleSize, 0);
      break;
      
    case kUIStateTimer:
      break;
      
    case kUIStateExit:
      ui_change_state(&ui_state_gallons);
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
      display_write_number(0, gUiCount * gUiSampleSize * 100 / 128, 2);
      break;
      
    case kUIStateTimer:
      break;
      
    case kUIStateExit:
      ui_change_state(&ui_state_elapsed);
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
      ui_change_state(&ui_state_count);
      break;
  }
}

void ui_configure_menu(void)
{
}

void ui_change_state(void (*state)(enum UIStateEvent))
{
  gUiSleepTime = millis() + kUiSleepTimeout;
  gUiStateFunc = state;
  (*gUiStateFunc)(kUIStateEnter);
}
