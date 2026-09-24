#pragma once
#include <stdint.h>

namespace usb {
struct HidMouseReport {
  uint8_t buttons;
  int16_t x;
  int16_t y;
  int8_t wheel;
  int8_t pan;
};

class Hid {
public:
  void init();
  void send_mouse(const HidMouseReport& r);
  void send_config(const uint8_t* data, uint16_t len);
  bool poll_out(uint8_t* out, uint16_t* len);
};
}
