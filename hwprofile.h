#ifndef BREWTALLY_HWPROFILE_H_
#define BREWTALLY_HWPROFILE_H_

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
#define DISPLAY_TIMER_CONFIG_A_REG        TCCR0A
#define DISPLAY_TIMER_CONFIG_B_REG        TCCR0B
#define DISPLAY_TIMER_INTERRUPT_MASK_REG  TIMSK
#define DISPLAY_TIMER_COMPARE_VALUE_REG   OCR0A

static const uint8_t kDisplayTimerMode = _BV(WGM01);
static const uint8_t kDisplayTimerInterruptMask = _BV(OCIE0A);
static const uint8_t kDisplayTimerCompareValue = 0x1f;
static const uint8_t kDisplayTimerPrescaler = (_BV(CS00) | _BV(CS01));


//Buttons input register
#define BUTTONS_INPUT_REG      PIND

//Buttons direction register
#define BUTTONS_DIR_REG      DDRD

//Buttons pin function mapping ([0] = Select, [1] = Sample)
static const uint8_t kButtonMasks[2] = { _BV(3), _BV(2) };

//Buttons  Interrupts
#define BUTTON_SELECT_VECTOR INT1_vect
#define BUTTON_SAMPLE_VECTOR INT0_vect
//MCUCR Values: INT0 (Change) + INT1 (Change)
static const uint8_t kButtonsInterruptSense = (_BV(ISC10) | _BV(ISC00));
//GIMSK Values: INT0 Enable + INT1 Enable
static const uint8_t kButtonsInterruptEn = (_BV(INT0) | _BV(INT1));

#endif
