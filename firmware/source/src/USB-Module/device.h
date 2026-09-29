#pragma once
#include <stdint.h>
#define USB_EP_MOUSE 2
#define USB_EP_KBD 3
#ifdef __cplusplus
extern "C"{
#endif
void usb_device_init(void);
void usb_device_poll(void);
uint8_t usb_device_sof(void);
uint8_t usb_device_configured(void);
uint8_t usb_ep_ready(uint8_t ep);
uint8_t usb_ep_send(uint8_t ep,const uint8_t *data,uint8_t len);
#ifdef __cplusplus
}
#endif