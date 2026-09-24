#pragma once
#include <stdint.h>

namespace mcu {
class Clock {
public:
  void init();
  void set_mhz(uint32_t mhz);
  uint32_t get_mhz() const;
  uint32_t micros() const;
  uint32_t millis() const;
  void delay_us(uint32_t us);
private:
  uint32_t mhz_;
};
}
