#include "usb_hid.h"
#include <Arduino.h>
#include <Mouse.h>

void usb_hid_init(void) {
  Mouse.begin();
}

void usb_hid_send_report(uint8_t buttons) {
  if (buttons & 0x01) {
    if (!Mouse.isPressed(MOUSE_LEFT)) {
      Mouse.press(MOUSE_LEFT);
    }
  } else {
    if (Mouse.isPressed(MOUSE_LEFT)) {
      Mouse.release(MOUSE_LEFT);
    }
  }

  if (buttons & 0x02) {
    if (!Mouse.isPressed(MOUSE_RIGHT)) {
      Mouse.press(MOUSE_RIGHT);
    }
  } else {
    if (Mouse.isPressed(MOUSE_RIGHT)) {
      Mouse.release(MOUSE_RIGHT);
    }
  }
}