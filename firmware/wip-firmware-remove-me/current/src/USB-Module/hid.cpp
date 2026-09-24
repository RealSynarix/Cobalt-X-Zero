#include "hid.h"
#include "device.h"

namespace usb {
void Hid::init() {}
void Hid::send_mouse(const HidMouseReport& r) {
  uint8_t buf[8];
  buf[0] = r.buttons;
  buf[1] = (uint8_t)(r.x & 0xFF);
  buf[2] = (uint8_t)(r.x >> 8);
  buf[3] = (uint8_t)(r.y & 0xFF);
  buf[4] = (uint8_t)(r.y >> 8);
  buf[5] = (uint8_t)r.wheel;
  buf[6] = (uint8_t)r.pan;
  buf[7] = 0;
  (void)buf;
}
void Hid::send_config(const uint8_t* data, uint16_t len) { (void)data; (void)len; }
bool Hid::poll_out(uint8_t* out, uint16_t* len) { (void)out; (void)len; return false; }
}
