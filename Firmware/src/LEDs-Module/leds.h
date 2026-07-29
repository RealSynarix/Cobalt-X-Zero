#ifndef LEDS_H
#define LEDS_H
#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif
void leds_init(void);
void leds_tick(void);
#ifdef __cplusplus
}
#endif
#endif
