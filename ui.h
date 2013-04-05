#ifndef BREWTALLY_UI_H_
#define BREWTALLY_UI_H_

#include "settings.h"

void ui_setup(struct BrewTallySettings *settings);
void ui_init(struct BrewTallySettings *settings);
void ui_update(void);

#endif