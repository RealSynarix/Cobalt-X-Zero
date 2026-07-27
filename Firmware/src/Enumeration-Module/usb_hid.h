#ifndef USB_HID_H
#define USB_HID_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void usb_hid_init(void);
void usb_hid_send_report(uint8_t buttons, int8_t wheel);
#ifdef __cplusplus
}
#endif
#endif
