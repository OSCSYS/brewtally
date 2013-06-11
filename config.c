#include "config.h"

#include <avr/eeprom.h>

#define CONFIG_FINGER_PRINT        136
#define CONFIG_DEFAULT_COUNT       0
#define CONFIG_DEFAULT_SAMPLE_SIZE 4
#define CONFIG_DEFAULT_BRIGHTNESS  15

uint8_t EEMEM gConfigFingerPrint = CONFIG_FINGER_PRINT;
uint16_t EEMEM gConfigCount = CONFIG_DEFAULT_COUNT;
uint8_t EEMEM gConfigSampleSize = CONFIG_DEFAULT_SAMPLE_SIZE;
uint8_t EEMEM gConfigBrightness = CONFIG_DEFAULT_BRIGHTNESS;

uint8_t config_init(void)
{
  if (eeprom_read_byte(&gConfigFingerPrint) == CONFIG_FINGER_PRINT)
    return 0;
  config_set_count(CONFIG_DEFAULT_COUNT);
  config_set_sample_size(CONFIG_DEFAULT_SAMPLE_SIZE);
  config_set_brightness(CONFIG_DEFAULT_BRIGHTNESS);
  eeprom_update_byte(&gConfigFingerPrint, CONFIG_FINGER_PRINT);
  return 1;
}

uint16_t config_get_count(void)
{
  return eeprom_read_word(&gConfigCount);
}

void config_set_count(uint16_t count)
{
  eeprom_update_word(&gConfigCount, count);
}

uint8_t config_get_sample_size(void)
{
  return eeprom_read_byte(&gConfigSampleSize);
}

void config_set_sample_size(uint8_t sampleSize)
{
  eeprom_update_byte(&gConfigSampleSize, sampleSize);
}

uint8_t config_get_brightness(void)
{
  return eeprom_read_byte(&gConfigBrightness);
}

void config_set_brightness(uint8_t brightness)
{
  eeprom_update_byte(&gConfigBrightness, brightness);
}