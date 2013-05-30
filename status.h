#ifndef BREWTALLY_STATUS_H_
#define BREWTALLY_STATUS_H_

#include <stdint.h>
#include <avr/io.h> 
 
//Function Declarations
void status_init(void);
void status_set(void);
void status_clear(void);
void status_toggle(void);

#endif