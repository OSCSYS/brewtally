#include "display.h"

#include <avr/interrupt.h>
#include <util/atomic.h> 

#include "hwprofile.h"

//Character definitions (PORT BIT TO SEGMENT MAP: ABCDEFGH)
static const uint8_t kCharTable[] = { 0xfc, //0
                                      0x60, //1
                                      0xda, //2
                                      0xf2, //3
                                      0x66, //4
                                      0xb6, //5
                                      0xbe, //6
                                      0xe0, //7
                                      0xfe, //8
                                      0xf6, //9
                                      0xee, //A
                                      0x3e, //b
                                      0x9c, //C (alt. c 0xc8)
                                      0x7a, //d
                                      0x9e, //E
                                      0x8e, //F
                                      0xbc, //G
                                      0x6e, //H (alt. h 0x9a)
                                      0x0c, //i
                                      0x78, //J
                                      0x00, //K NOT SUPPORTED
                                      0x1c, //L
                                      0x00, //M NOT SUPPORTED
                                      0x2a, //n (alt. N 0xec)
                                      0xfc, //o (alt. 0 0xd8)
                                      0xce, //P
                                      0xe6, //q
                                      0x0A, //r
                                      0xb6, //S (dupe of 5)
                                      0x1e, //t
                                      0x7c, //U (alt. u 0xd0)
                                      0x00, //V NOT SUPPORTED
                                      0x00, //W NOT SUPPORTED
                                      0x00, //X NOT SUPPORTED
                                      0x76, //y
                                      0x00  //Z NOT SUPPORTED
};

static const uint8_t kCharDecimal = 0x01;

#define DISPLAY_FRAME_COUNT 3
static const uint16_t kDisplayFrameTime = 2000;

//Global Char values for timer interrupt ISRs
static uint8_t gDisplayCharBuffer[DISPLAY_FRAME_COUNT][DISPLAY_CHAR_COUNT];
static uint8_t gActiveFrames = 0;
static volatile uint8_t gFrameCursor = 0;
static volatile uint8_t gDisplayCharCursor = 0;
static volatile uint32_t gDisplayFrameTimestamp = 0;

//Global tick and millis counter
static volatile uint8_t gDisplayTick = 0;
static volatile uint32_t gDisplayMillis = 0;

void display_init(void)
{
  DISPLAY_CHAR_SELECT_DIR_REG |= kDisplayCharSelectPinMask;       //Enable Digit Select Pins as outputs
  DISPLAY_CHAR_DIR_REG |= kDisplayCharPinMask;                    //Enable Char pins as outputs
  DISPLAY_CHAR_SELECT_OUTPUT_REG &= ~kDisplayCharSelectPinMask;
  DISPLAY_CHAR_OUTPUT_REG = kDisplayCharPinMask;
  
  DISPLAY_TIMER_MODE_REG |= kDisplayTimerMode;                    //Configure timer for CTC mode 
  DISPLAY_TIMER_INTERRUPT_MASK_REG |= kDisplayTimerInterruptMask; //Enable timer interrupt
  sei();                                                          //Enable global interrupts 
  DISPLAY_TIMER_COMPARE_VALUE_REG = kDisplayTimerCompareValue;    //Set compare value for a compare rate of 1kHz 
  DISPLAY_TIMER_PRESCALER_REG |= kDisplayTimerPrescaler;          //Set timer prescaler
  display_clear();
}

void display_clear(void)
{
  for (uint8_t frame = DISPLAY_FRAME_COUNT; frame; --frame) {
    for (uint8_t i = DISPLAY_CHAR_COUNT; i; --i) {
      gDisplayCharBuffer[frame][i] = 0;
    }
  }
  gActiveFrames = 0;
}

void display_write_number(uint8_t frame, uint16_t number)
{
  if (number > DISPLAY_MAX_NUMBER)
    return;
  if (frame >= DISPLAY_FRAME_COUNT)
    return;
 
  for(uint8_t i = 1; i <= DISPLAY_DIGIT_COUNT; ++i) {
    gDisplayCharBuffer[frame][i] = (number || (i == 1)) ? kCharTable[number % 10] : 0;
    number /= 10;
  }
  
  if (frame >= gActiveFrames)
    gActiveFrames = frame + 1;  
  display_frame_focus(frame);
}

void display_write_decimal(uint8_t frame, uint8_t digit)
{
  if (digit <= DISPLAY_DIGIT_COUNT)
    gDisplayCharBuffer[frame][digit] |= kCharDecimal;
}

void display_write_string(uint8_t frame, const char *text)
{
  if (frame >= DISPLAY_FRAME_COUNT)
    return;
  uint8_t cursor = DISPLAY_DIGIT_COUNT;

  while(*text && cursor) {
    uint8_t bmp = 0x00;
    if(*text >= '0' && *text <= '9')
      bmp = kCharTable[*text - '0'];       //Handle Digits
    else if (*text >= 'A' && *text <= 'Z')
      bmp = kCharTable[*text - 55];        //Handle A-U
    else if (*text >= 'a' && *text <= 'z')
      bmp = kCharTable[*text - 87];        //Handle A-U
    gDisplayCharBuffer[frame][cursor--] = bmp;
    ++text;
  }
  if (frame >= gActiveFrames)
    gActiveFrames = frame + 1;  
  display_frame_focus(frame);
}

void display_frame_focus(uint8_t frame) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    gFrameCursor = frame;
    gDisplayFrameTimestamp = gDisplayMillis;
  }
}

#ifdef DISPLAY_ENABLE_MILLIS
uint32_t millis(void)
{
  unsigned long ms;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ms = gDisplayMillis;
  }
  return ms;
}
#endif

ISR(DISPLAY_TIMER_VECTOR) 
{
  if (++gDisplayTick & 1)
    ++gDisplayMillis;    //Increment global millis counter on every other tick
  
  DISPLAY_CHAR_SELECT_OUTPUT_REG &= ~kDisplayCharSelectPinMask; //Bring all digit select pins low
  
  if(!gActiveFrames)    //Short-circuit when no screens active
    return; 
    
  if (gDisplayCharCursor < DISPLAY_CHAR_COUNT) { //Ignore dummy cycles
    DISPLAY_CHAR_OUTPUT_REG = ~(gDisplayCharBuffer[gFrameCursor][gDisplayCharCursor]); //Write char value
    DISPLAY_CHAR_SELECT_OUTPUT_REG |= kDisplayCharSelect[gDisplayCharCursor]; //Bring current digit select pin high
  }
  
  if (gDisplayMillis - gDisplayFrameTimestamp > kDisplayFrameTime) { //Handle frame change
    gFrameCursor++;
    if (gFrameCursor >= gActiveFrames) //Frame roll over
      gFrameCursor = 0;
    gDisplayFrameTimestamp = gDisplayMillis;
  }
  
  if (++gDisplayCharCursor == DISPLAY_CHAR_COUNT + 16)  //Char scan with dummy cycles to reduce power consumption
    gDisplayCharCursor = gDisplayCharBuffer[gFrameCursor][0] ? 0 : 1; //Skip colon index 0 if not active
}