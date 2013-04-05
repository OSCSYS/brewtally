#include "settings.h"

#include <util/crc16.h>
#include <avr/eeprom.h>

struct BrewTallySettings EEMEM eepromSettings;

uint8_t settings_crc(struct BrewTallySettingsData *data);

uint8_t settings_init(struct BrewTallySettings *settings)
{
  if (
    settings->header.version == kSettingsVersion &&
    settings->header.size == sizeof(settings) &&
    settings->header.crc == settings_crc(&settings->data)
  )
    return 0;

  settings->header.version = kSettingsVersion;
  settings->header.size = sizeof(settings);
  //TODO: Set settings->data defaults
  return(1);
}

void settings_load(struct BrewTallySettings *settings)
{
  eeprom_read_block((void*)settings, (const void*)&eepromSettings, sizeof(settings)); 
}

void settings_save(struct BrewTallySettings *settings)
{
  settings->header.crc = settings_crc(&settings->data);
  eeprom_update_block((void*)settings, (void*)&eepromSettings, sizeof(eepromSettings)); 
}

uint8_t settings_crc(struct BrewTallySettingsData *data)
{
  uint8_t crc = 0;
  uint8_t* chunk = (uint8_t*) data;
  
  for (uint8_t i = 0; i < sizeof(data); i++)
    crc = _crc_ibutton_update(crc, *chunk++);
  return crc;
}