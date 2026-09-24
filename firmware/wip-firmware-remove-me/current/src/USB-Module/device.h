#pragma once
#include <stdint.h>
#include "descriptors.h"

namespace usb {
class Device {
public:
  void init();
  void process();
  bool is_configured() const;
  void send_report(uint8_t id, const uint8_t* data, uint16_t len);
private:
  bool configured_;
};
}
