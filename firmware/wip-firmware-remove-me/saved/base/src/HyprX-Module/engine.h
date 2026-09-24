#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void hyprx_init(void);
uint8_t hyprx_update(uint8_t buttons, int32_t *delta);
uint8_t hyprx_tick(void);
uint8_t hyprx_active(void);
#ifdef __cplusplus
}
#endif
