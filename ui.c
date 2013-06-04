#include "ui.h"

#include <stdint.h>
#include <string.h>
#include <avr/io.h> 

#include "buttons.h"
#include "config.h"
#include "display.h"
#include "throbber.h"

static const uint32_t kUiSleepTimeout = 7200000UL;
static const uint8_t kUiMaxSampleSize = 22;


static uint32_t gUiSampleTime = 0;
static uint32_t gUiSleepTime = 0;

enum UIStateEvent {
  kUIStateEnter,
  kUIStateUpdate,
  kUIStateTimer,
  kUIStateExit
};

struct MenuItem {
  char label[5];
  uint8_t (*menuFunc)(void);
};

void ui_state_count(enum UIStateEvent);
void ui_state_ounces(enum UIStateEvent);
void ui_state_gallons(enum UIStateEvent);
void ui_state_elapsed(enum UIStateEvent);
void ui_configure_menu(void);
void ui_change_state(void (*state)(enum UIStateEvent));
void ui_menu(char title[5],struct MenuItem *menu, uint8_t size);
uint8_t ui_menu_exit(void);
uint8_t ui_configure_brightness(void);
uint8_t ui_configure_size(void);
uint8_t ui_configure_count(void);
uint8_t ui_clear_count(void);

void (*gUiStateFunc)(enum UIStateEvent) = 0;

void ui_init(void)
{
  config_init();
  throbber_init();
  display_init();
  display_set_brightness(config_get_brightness());
  buttons_init();
  ui_change_state(&ui_state_count);
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
  uint16_t lastCount = config_get_count();
  static uint32_t refreshTime = 0;
  if (timestamp > gUiSleepTime) {
    throbber_init();
    display_clear();
    gUiStateFunc = 0;
    return;
  }
  if (button_short(kButtonSample)) {
    config_set_count(lastCount + 1);
    gUiSampleTime = timestamp;
    throbber_set(gUiSampleTime);
    gUiSleepTime = timestamp + kUiSleepTimeout;
  }
  if (button_long(kButtonSample) && lastCount)
      config_set_count(lastCount - 1);
  if (button_short(kButtonSelect))
    (*gUiStateFunc)(kUIStateExit);
  if (button_long(kButtonSelect)) {
    ui_configure_menu();
    (*gUiStateFunc)(kUIStateEnter);
  }
  if (lastCount != config_get_count())
    (*gUiStateFunc)(kUIStateUpdate);
  if ((timestamp - refreshTime) / 1000) {
    (*gUiStateFunc)(kUIStateTimer);
    refreshTime = timestamp;
  }
  throbber_update(timestamp);
}

void ui_change_state(void (*state)(enum UIStateEvent))
{
  gUiSleepTime = millis() + kUiSleepTimeout;
  gUiStateFunc = state;
  display_clear();
  (*gUiStateFunc)(kUIStateEnter);
}

void ui_state_count(enum UIStateEvent event)
{
  switch (event) {
    case kUIStateEnter:
      ui_state_count(kUIStateUpdate);
      display_write_string(1, " Cnt");
      break;
      
    case kUIStateUpdate:
      display_write_number(0, config_get_count(), 0);
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
      display_write_number(0, config_get_count() * config_get_sample_size(), 0);
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
      display_write_number(0, (uint32_t)config_get_count() * config_get_sample_size() * 100 / 128, 2);
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
  struct MenuItem configMenu[] = {
    { "dISP", &ui_configure_brightness },
    { "SIZE", &ui_configure_size },
    { " Cnt", &ui_configure_count },
    { "EHit", &ui_menu_exit },
  };
  ui_menu("CFG", &configMenu[0], sizeof(configMenu) / sizeof(configMenu[0]));
}

void ui_menu(char title[5],struct MenuItem *menu, uint8_t size)
{
  uint8_t cursor = 0;
  uint8_t lastCursor = 1;
  while(1) {
    if (button_short(kButtonSelect)) {
      ++cursor;
      if (cursor == size)
        cursor = 0;
    }
    if (button_short(kButtonSample)) {
      if (menu[cursor].menuFunc())
        return;
      lastCursor = cursor + 1;
    }
    if (cursor != lastCursor) {
      display_clear();
      display_write_string(1, title);
      display_write_string(0, menu[cursor].label);
      lastCursor = cursor;
    }
  }
}

uint8_t ui_menu_exit(void)
{
  return 1;
}

uint8_t ui_configure_brightness(void)
{
  uint8_t brightness = config_get_brightness();
  uint8_t lastValue = brightness + 1;
  while(1) {
    if (button_short(kButtonSelect)) {
      ++brightness;
      if (brightness > kDisplayMaxBrightness)
        brightness = 0;
    }
    if (button_short(kButtonSample)) {
      config_set_brightness(brightness);
      return 0;
    }
    if (brightness != lastValue) {
      display_clear();
      display_write_string(1, "dISP");
      display_write_number(0, brightness, 0);
      lastValue = brightness;
      display_set_brightness(brightness);
    }
  }
}

uint8_t ui_configure_size(void)
{
  uint8_t size = config_get_sample_size();
  uint8_t lastSize = size + 1;
  while(1) {
    if (button_short(kButtonSelect)) {
      ++size;
      if (size > kUiMaxSampleSize)
        size = 1;
    }
    if (button_short(kButtonSample)) {
      config_set_sample_size(size);
      return 0;
    }
    if (size != lastSize) {
      display_clear();
      display_write_string(1, "Size");
      display_write_number(0, size, 0);
      lastSize = size;
    }
  }
}

uint8_t ui_configure_count(void)
{
  struct MenuItem clearMenu[] = {
    { "CnCL", &ui_menu_exit },
    { " CLr", &ui_clear_count }
  };
  ui_menu("Cnt", &clearMenu[0], sizeof(clearMenu) / sizeof(clearMenu[0]));
  return 0;
}

uint8_t ui_clear_count(void)
{
  config_set_count(0);
  return 1;
}

