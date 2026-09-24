#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void sensor_init(void); void sensor_fast_tick(void); void sensor_tick(void);
uint8_t sensor_pop(int16_t *dx,int16_t *dy);
#ifdef __cplusplus
}
#endif
extern const uint16_t MOUSE_DPI; extern const uint8_t MOUSE_LOD_MM;
