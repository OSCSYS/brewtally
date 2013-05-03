#ifndef BREWTALLY_HWPROFILE_H_
#define BREWTALLY_HWPROFILE_H_

//Status output registers
#define STATUS_OUTPUT_REG   PORTA

//Status direction registers
#define STATUS_DIR_REG      DDRA

//Status pin bitmask
static const uint8_t kStatusPinMask  = 0x02;

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
#define DISPLAY_CHAR_COUNT 5
#define DISPLAY_DIGIT_COUNT 4
#define DISPLAY_DIGIT_OFFSET 1
#define DISPLAY_MAX_NUMBER 9999
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


//Buttons input register
#define BUTTONS_INPUT_REG      PIND
#define BUTTONS_PULLUP_REG     PORTD

//Buttons direction register
#define BUTTONS_DIR_REG      DDRD

enum ButtonMask {
  kButtonSelect = _BV(3),
  kButtonSample = _BV(2)
};

//Buttons  Interrupts
#define BUTTON_TIMER_VECTOR              TIMER1_COMPA_vect
#define BUTTON_TIMER_PRESCALER_REG       TCCR1B
#define BUTTON_TIMER_MODE_REG            TCCR1B
#define BUTTON_TIMER_INTERRUPT_MASK_REG  TIMSK
#define BUTTON_TIMER_COMPARE_VALUE_REG   OCR1A
static const uint8_t kButtonTimerMode = _BV(WGM12);
static const uint8_t kButtonTimerInterruptMask = _BV(OCIE1A);
static const uint16_t kButtonTimerCompareValue = 0x9C40;
static const uint8_t kButtonTimerPrescaler = _BV(CS10);

#endif
