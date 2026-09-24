#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void usb_device_init(void);
uint8_t usb_device_sof(void);
#ifdef __cplusplus
}
#endif
