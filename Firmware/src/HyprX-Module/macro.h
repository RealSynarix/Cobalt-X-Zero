#ifndef MACRO_H
#define MACRO_H
#include <stdint.h>
void macro_init(void);
uint8_t macro_update(uint8_t buttons, int32_t *delta);
uint8_t macro_frame_tick(void);
uint8_t macro_is_hyprx_active(void);
#endif
