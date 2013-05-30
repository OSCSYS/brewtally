#include "display.h"

#include <avr/interrupt.h>
#include <avr/io.h> 
#include <avr/pgmspace.h>
#include <util/atomic.h> 

//Character definitions (PORT BIT TO SEGMENT MAP: ABCDEFGH)
static const uint8_t PROGMEM kCharTable[] = { 0xfc, //0
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
                                              0xda  //Z (dupe of 2)
};

static const uint8_t kCharDecimal = 0x01;
static const uint8_t kCharColonOn = 0xff;
static const uint8_t kCharColonOff = 0x00;

#define DISPLAY_CHAR_COUNT 5
#define DISPLAY_DIGIT_COUNT 4
#define DISPLAY_DIGIT_OFFSET 1
#define DISPLAY_MAX_NUMBER 9999
#define DISPLAY_FRAME_COUNT 3
static const uint16_t kDisplayFrameTime = 2000;

static const uint8_t kDisplayCharColon = 0;

//Display output registers
#define DISPLAY_CHAR_SELECT_OUTPUT_REG PORTD
#define DISPLAY_CHAR_OUTPUT_REG        PORTB

//Display direction registers
#define DISPLAY_CHAR_SELECT_DIR_REG   DDRD
#define DISPLAY_CHAR_DIR_REG          DDRB

//Display pin bitmasks
static const uint8_t kDisplayCharSelectPinMask  = 0x73;
static const uint8_t kDisplayCharPinMask         = 0xff;

//Display char select bit map
static const uint8_t kDisplayCharSelect[DISPLAY_CHAR_COUNT] = {_BV(0), _BV(1), _BV(4), _BV(5), _BV(6)};

//Display Timer Configuration
#define DISPLAY_TIMER_VECTOR              TIMER0_COMPA_vect
#define DISPLAY_TIMER_MODE_REG            TCCR0A
#define DISPLAY_TIMER_PRESCALER_REG       TCCR0B
#define DISPLAY_TIMER_INTERRUPT_MASK_REG  TIMSK
#define DISPLAY_TIMER_COMPARE_VALUE_REG   OCR0A
static const uint8_t kDisplayTimerMode = _BV(WGM01);
static const uint8_t kDisplayTimerInterruptMask = _BV(OCIE0A);
static const uint8_t kDisplayTimerCompareValue = 0x1f;
static const uint8_t kDisplayTimerPrescaler = (_BV(CS00) | _BV(CS01));


//Global Char values for timer interrupt ISRs
static volatile uint8_t gDisplayCharBuffer[DISPLAY_FRAME_COUNT][DISPLAY_CHAR_COUNT];
static volatile uint8_t gActiveFrames = 0;
static volatile uint8_t gFrameCursor = 0;
static volatile uint8_t gDisplayCharCursor = 0;
static volatile uint32_t gDisplayFrameTimestamp = 0;

//Global millis counter
static volatile uint32_t gDisplayMillis = 0;

//Helper function declarations
static uint16_t display_pop_time(uint32_t *timeValue);

void display_init(void)
{
  DISPLAY_CHAR_SELECT_DIR_REG |= kDisplayCharSelectPinMask;       //Enable Digit Select Pins as outputs
  DISPLAY_CHAR_DIR_REG |= kDisplayCharPinMask;                    //Enable Char pins as outputs
  DISPLAY_CHAR_SELECT_OUTPUT_REG &= ~kDisplayCharSelectPinMask;
  DISPLAY_CHAR_OUTPUT_REG = kDisplayCharPinMask;
  
  DISPLAY_TIMER_MODE_REG |= kDisplayTimerMode;                    //Configure timer for CTC mode 
  DISPLAY_TIMER_COMPARE_VALUE_REG = kDisplayTimerCompareValue;    //Set compare value for a compare rate of 1kHz 
  DISPLAY_TIMER_PRESCALER_REG |= kDisplayTimerPrescaler;          //Set timer prescaler
  display_clear();
  DISPLAY_TIMER_INTERRUPT_MASK_REG |= kDisplayTimerInterruptMask; //Enable timer interrupt
  sei();                                                          //Enable global interrupts 
}

void display_clear(void)
{
  for (uint8_t f = 0; f < DISPLAY_FRAME_COUNT; f++) {
    for (uint8_t c = 0; c < DISPLAY_CHAR_COUNT; c++) {
      gDisplayCharBuffer[f][c] = 0;
    }
  }
  gActiveFrames = 0;
}

void display_write_number(uint8_t frame, uint16_t number, uint8_t precision)
{
  if (frame >= DISPLAY_FRAME_COUNT
      || number > DISPLAY_MAX_NUMBER
      || precision >= DISPLAY_DIGIT_COUNT)
    return;

  uint8_t spacePad = precision ? precision + 2: 2; //Set minimum number of displayed digits
  for(uint8_t i = 1; i <= DISPLAY_DIGIT_COUNT; ++i) {
    uint8_t digit = number % 10;
    gDisplayCharBuffer[frame][i] = (number || i < spacePad) ? pgm_read_byte(&kCharTable[digit]) : 0;
    number /= 10;
  }
  if (precision)
    gDisplayCharBuffer[frame][precision + 1] |= kCharDecimal;
  gDisplayCharBuffer[frame][kDisplayCharColon] = kCharColonOff;
  if (frame >= gActiveFrames)
    gActiveFrames = frame + 1;  
  display_frame_focus(frame);
}

void display_write_string(uint8_t frame, const char *text)
{
  if (frame >= DISPLAY_FRAME_COUNT)
    return;

    uint8_t cursor = DISPLAY_DIGIT_COUNT;
  while(*text && cursor) {
    uint8_t offset = 0;
    if(*text >= '0' && *text <= '9')
      offset = '0';       //Handle Digits
    else if (*text >= 'A' && *text <= 'Z')
      offset = 55;        //Handle A-Z
    else if (*text >= 'a' && *text <= 'z')
      offset = 87;        //Handle a-z
    gDisplayCharBuffer[frame][cursor--] = offset ? pgm_read_byte(kCharTable + *text - offset) : 0;
    ++text;
  }
  gDisplayCharBuffer[frame][kDisplayCharColon] = kCharColonOff;
  if (frame >= gActiveFrames)
    gActiveFrames = frame + 1;  
  display_frame_focus(frame);
}

//Note this implementation assumes 4-digit display
void display_write_time(uint8_t frame, uint32_t timeValue)
{
  if (frame >= DISPLAY_FRAME_COUNT)
    return;

  timeValue /= 1000; //Convert to seconds
  //Time values as uint16_t to allow for scaling later
  uint16_t seconds = display_pop_time(&timeValue);
  uint16_t minutes = display_pop_time(&timeValue);
  uint16_t hours = display_pop_time(&timeValue);
  
  if (hours > 99) hours = 99;
  timeValue = hours ? hours * 100 + minutes : minutes * 100 + seconds;
  display_write_number(frame, timeValue, 3);     //Set max precision to force 0-padding
  gDisplayCharBuffer[frame][4] &= ~kCharDecimal; //Remove decimal created by precision
  gDisplayCharBuffer[frame][kDisplayCharColon] = kCharColonOn;
}

static uint16_t display_pop_time(uint32_t *timeValue)
{
  uint16_t returnValue = *timeValue % 60;
  *timeValue /= 60;
  return returnValue;
}

void display_frame_focus(uint8_t frame) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    gFrameCursor = frame;
    gDisplayFrameTimestamp = gDisplayMillis;
  }
}

uint32_t millis(void)
{
  unsigned long ms;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    ms = gDisplayMillis;
  }
  return ms;
}

ISR(DISPLAY_TIMER_VECTOR) 
{
  static uint8_t tick = 0;
  if (tick ^= 1) 
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
    gDisplayCharCursor = 0; //Skip colon index 0 if not active
}