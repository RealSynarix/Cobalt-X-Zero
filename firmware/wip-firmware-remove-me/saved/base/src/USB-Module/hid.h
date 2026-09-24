#pragma once
#include <stdint.h>
void hid_init(void);
void hid_send(uint8_t b,int8_t w,int16_t dx,int16_t dy);
