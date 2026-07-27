#include "usb_hid.h"
#include <Arduino.h>
#define private public
#include <Mouse.h>
#undef private

void usb_hid_init(void) {
  Mouse.begin();
}

void usb_hid_send_report(uint8_t buttons, int8_t wheel) {
  Mouse._buttons = buttons & 0x07;
  Mouse.move(0, 0, wheel);
}
