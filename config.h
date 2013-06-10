#ifndef BREWTALLY_CONFIG_H_
#define BREWTALLY_CONFIG_H_

#include <stdint.h>

uint8_t config_init(void);
uint16_t config_get_count(void);
void config_set_count(uint16_t count);
uint8_t config_get_sample_size(void);
void config_set_sample_size(uint8_t sampleSize);
uint8_t config_get_brightness(void);
void config_set_brightness(uint8_t brightness);
#endif