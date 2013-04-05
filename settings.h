#ifndef BREWTALLY_SETTINGS_H_
#define BREWTALLY_SETTINGS_H_

#include <stdint.h>

static const uint8_t kSettingsVersion = 1;

struct BrewTallySettingsHeader {
  uint8_t version;
  uint8_t size;
  uint8_t crc;
};

struct BrewTallySettingsData {
  
};

struct BrewTallySettings {
  struct BrewTallySettingsHeader header;
  struct BrewTallySettingsData data;
};

//Sets default values and returns 1 if settings are invalid
uint8_t settings_init(struct BrewTallySettings *settings);
void settings_load(struct BrewTallySettings *settings);
void settings_save(struct BrewTallySettings *settings);

#endif