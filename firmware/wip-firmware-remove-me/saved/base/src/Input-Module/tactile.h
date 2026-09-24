#pragma once
#include <stdint.h>
typedef struct { uint8_t buttons; int8_t wheel; } tactile_report_t;
#ifdef __cplusplus
extern "C" {
#endif
void tactile_init(void);
void tactile_tick(void);
uint8_t tactile_pop(tactile_report_t *out);
#ifdef __cplusplus
}
#endif
