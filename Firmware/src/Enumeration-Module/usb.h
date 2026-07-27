#ifndef USB_H
#define USB_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void usb_init(void);
uint8_t usb_sof_detected(void);
#ifdef __cplusplus
}
#endif
#endif
