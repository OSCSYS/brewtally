#ifndef BREWTALLY_DISPLAY_H_
#define BREWTALLY_DISPLAY_H_

#include <stdint.h>
#include <avr/io.h> 

//#define DISPLAY_ENABLE_MILLIS

//Function declarations
void display_init(void);

//Reset buffer and clear frame count
void display_clear(void);

//Write a number
void display_write_number(uint8_t frame, uint16_t number);

//Adds a decimal point to an existing digit position
//Must be called after each new write to a frame
void display_write_decimal(uint8_t frame, uint8_t digit);

//Write a string (limited to display size, limited char support 0-9, A-U)
void display_write_string(uint8_t frame, const char* text);

void display_frame_focus(uint8_t frame);

#ifdef DISPLAY_ENABLE_MILLIS
uint32_t millis(void);
#endif

#endif
