#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif
typedef struct{
int16_t dx;
int16_t dy;
uint8_t squal;
uint8_t lifted;
}sf_packet_t;
typedef struct{
uint8_t squal_min;
uint16_t dpi;
uint8_t snap_en;
uint8_t snap_strength;
float a_min;
}sf_params_t;
void sf_reset(void);
uint8_t sf_frame(const sf_packet_t *p,uint8_t n,const sf_params_t *prm,float *ox,float *oy);
#ifdef __cplusplus
}
#endif
