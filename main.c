#include "settings.h"
#include "ui.h"

int main(void)
{
  struct BrewTallySettings systemSettings;
  settings_load(&systemSettings);
  
  //Check settings validity launching settings UI Menu if necessary
  if (settings_init(&systemSettings))
    settings_save(&systemSettings);

  ui_init(&systemSettings);
    
  while (1) {
    ui_update();
  }
}

