#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif
void sensor_init(void);
void sensor_task(void);
void sensor_take(int16_t *dx,int16_t *dy);
uint8_t sensor_ready(void);
#ifdef __cplusplus
}
#endif
