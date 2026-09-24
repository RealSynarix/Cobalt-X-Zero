#pragma once
#include <stdint.h>
void sensor_init();
uint8_t sensor_raw(int16_t *dx,int16_t *dy,uint8_t *squal,uint8_t *motion, uint8_t *raw_sum, uint8_t *max_raw, uint8_t *min_raw, uint16_t *shutter);
