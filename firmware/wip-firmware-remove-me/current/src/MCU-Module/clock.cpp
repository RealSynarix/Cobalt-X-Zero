#include "clock.h"
#include <Arduino.h>
#include "src/Config-Module/config.h"

namespace mcu {
void Clock::init() {
  mhz_ = cfg::cfg_get()->clock_mhz;
  if (mhz_ < 48) mhz_ = 48;
  if (mhz_ > 240) mhz_ = 240;
}
void Clock::set_mhz(uint32_t mhz) {
  if (mhz < 48 || mhz > 240) return;
  mhz_ = mhz;
}
uint32_t Clock::get_mhz() const { return mhz_; }
uint32_t Clock::micros() const { return ::micros(); }
uint32_t Clock::millis() const { return ::millis(); }
void Clock::delay_us(uint32_t us) { ::delayMicroseconds(us); }
}
