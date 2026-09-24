#pragma once
#include <stdint.h>

#define USB_VID 0x1209
#define USB_PID 0xC0BA
#define USB_MANUFACTURER_STRING "Generic"
#define USB_PRODUCT_STRING "HyprX Device"
#define USB_SERIAL_STRING "HX00000001"
#define USB_EP0_SIZE 64
#define USB_HID_EP_SIZE 64
#define USB_HID_INTERVAL 1
#define USB_DEVICE_CLASS 0x00
#define USB_DEVICE_SUBCLASS 0x00
#define USB_DEVICE_PROTOCOL 0x00

extern const uint8_t usb_device_desc[];
extern const uint8_t usb_config_desc[];
extern const uint8_t usb_hid_report_desc[];
extern const uint16_t usb_hid_report_size;
