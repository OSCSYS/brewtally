#ifndef BREWTALLY_THROBBER_H_
#define BREWTALLY_THROBBER_H_

#include <stdint.h>

void throbber_init(void);
void throbber_set(uint32_t timestamp);
void throbber_update(uint32_t now);

#endif