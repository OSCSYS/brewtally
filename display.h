#ifndef BREWTALLY_DISPLAY_H_
#define BREWTALLY_DISPLAY_H_

#include <stdint.h>

static const uint8_t kDisplayMaxBrightness = 25;

//#define DISPLAY_ENABLE_MILLIS

//Function declarations
void display_init(void);

//Set brightness value between 0 (minimum bright) and 25 (full bright)
void display_set_brightness(uint8_t brightness);

//Reset buffer and clear frame count
void display_clear(void);

//Write a number
void display_write_number(uint8_t frame, uint16_t number, uint8_t precision);

//Write a string (limited to display size, limited char support 0-9, A-U)
void display_write_string(uint8_t frame, const char* text);

//Write timeValue in milliseconds as HH:MM or MM:SS
void display_write_time(uint8_t frame, uint32_t timeValue);

void display_frame_focus(uint8_t frame);

uint32_t millis(void);

#endif
