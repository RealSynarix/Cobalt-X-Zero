#ifndef MACRO_H
#define MACRO_H

#include <stdint.h>

void macro_init(void);
uint8_t macro_update(uint8_t buttons, int32_t *delta);

#endif