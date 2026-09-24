#include "device.h"
#include <Arduino.h>

namespace usb {
const uint8_t usb_device_desc[] = {
  18, 1, 0x00, 0x02, 0, 0, 0, USB_EP0_SIZE,
  0x09, 0x12, 0xBA, 0xC0, 0x00, 0x01, 1, 2, 3, 1
};
const uint8_t usb_config_desc[] = {
  9, 2, 34, 0, 1, 1, 0, 0x80, 50,
  9, 4, 0, 0, 1, 3, 0, 0, 0,
  9, 0x21, 0x11, 1, 0, 1, 0x22, 64, 0,
  7, 5, 0x81, 3, 64, 0, USB_HID_INTERVAL
};
const uint8_t usb_hid_report_desc[] = {
  0x06, 0x00, 0xFF, 0x09, 0x01, 0xA1, 0x01, 0x85, 0x01,
  0x09, 0x02, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08,
  0x95, 0x40, 0x81, 0x02, 0x85, 0x02, 0x09, 0x03,
  0x95, 0x40, 0x91, 0x02, 0xC0
};
const uint16_t usb_hid_report_size = sizeof(usb_hid_report_desc);

void Device::init() { configured_ = false; }
void Device::process() { if (!configured_) configured_ = true; }
bool Device::is_configured() const { return configured_; }
void Device::send_report(uint8_t id, const uint8_t* data, uint16_t len) { (void)id; (void)data; (void)len; }
}
