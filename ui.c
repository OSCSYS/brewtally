#include "ui.h"

#include <stdint.h>
#include <string.h>

#include "buttons.h"
#include "calcs.h"
#include "display.h"
#include "hwprofile.h"

enum UiState {
  kUiStateOff,
  kUiStateNumStates
};

uint16_t gSampleCount = 0;

void ui_state_enter(enum UiState state);
uint8_t ui_setup_reset(struct BrewTallySettings *settings);
uint8_t ui_setup_save(struct BrewTallySettings *settings);
uint8_t ui_get_yes_no(uint8_t value, char displayYes[], char displayNo[]);

struct menuItem {
  char title[4];
  uint8_t (*menuFunc)(struct BrewTallySettings*);
};

static const struct menuItem kSettingsMenu[] = {
  {"rSt", ui_setup_reset},
  {"SEt", ui_setup_save}
};

static struct BrewTallySettings *gUiSettings;
static enum UiState gUiState = kUiStateOff;

void ui_init(struct BrewTallySettings *settings)
{
  gUiSettings = settings;
  ui_state_enter(kUiStateOff);
  display_init();
  buttons_init();
}

static uint32_t gUiNextUpdate;
static uint16_t gUiCount;
void ui_update()
{
  if (millis() > gUiNextUpdate ) {
    gUiNextUpdate += 1000;
    gUiCount++;
    if (gUiCount > 9999)
      gUiCount = 0;
    display_write_number(9999 - gUiCount, 0);
  }
}

void ui_state_enter(enum UiState state)
{

}

void ui_setup(struct BrewTallySettings *settings)
{

}

uint8_t ui_setup_reset(struct BrewTallySettings *settings)
{
  if(ui_get_yes_no(0, "yES", " No")) {
    settings->header.size = 0; //Invalidate header
    settings_init(settings); //Initialize Settings
  }
  return 0;
}

uint8_t ui_setup_save(struct BrewTallySettings *settings)
{
  //Flag for Exit, Settings saved in main() initialization
  return 1;
}

uint8_t ui_get_yes_no(uint8_t value, char displayYes[], char displayNo[])
{
  return 0;
}