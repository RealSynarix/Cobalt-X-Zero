#include "usb_hid.h"
#include <Arduino.h>
#include <Mouse.h>

static uint8_t last_sent = 0;

void usb_hid_init(void) {
  Mouse.begin();
}

void usb_hid_send_report(uint8_t buttons, int8_t wheel) {
  if (buttons != last_sent) {
    if ((buttons & 0x01) != (last_sent & 0x01)) (buttons & 0x01) ? Mouse.press(MOUSE_LEFT) : Mouse.release(MOUSE_LEFT);
    if ((buttons & 0x02) != (last_sent & 0x02)) (buttons & 0x02) ? Mouse.press(MOUSE_RIGHT) : Mouse.release(MOUSE_RIGHT);
    if ((buttons & 0x04) != (last_sent & 0x04)) (buttons & 0x04) ? Mouse.press(MOUSE_MIDDLE) : Mouse.release(MOUSE_MIDDLE);
    last_sent = buttons;
  }

  if (wheel) {
    Mouse.move(0, 0, wheel);
  }
}