#include "sensor.h"
const uint16_t MOUSE_DPI=1000; const uint8_t MOUSE_LOD_MM=2;
void sensor_init(void){} void sensor_fast_tick(void){} void sensor_tick(void){}
uint8_t sensor_pop(int16_t *dx,int16_t *dy){ if(dx) *dx=0; if(dy) *dy=0; return 0; }
