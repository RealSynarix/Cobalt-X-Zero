#include "usb_hid.h"
#include <Arduino.h>
#include <Mouse.h>

void usb_hid_init(void) {
  Mouse.begin();
}