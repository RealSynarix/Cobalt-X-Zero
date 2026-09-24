#pragma once
#include <stdint.h>
#include <stdbool.h>

namespace hyprx {
struct ScrollState {
  int32_t accum;
  uint8_t div_shift;
  uint16_t hold_ticks;
  uint8_t pending;
  uint8_t target;
};

class Engine {
public:
  void init();
  void update();
  void feed_scroll(int8_t delta);
  void set_enabled(bool en);
  bool is_enabled() const;
  uint8_t get_pending() const;
  void reset();
private:
  bool enabled_;
  ScrollState state_;
  uint8_t pending_max_;
  bool scroll_to_click_;
  uint8_t activation_;
  uint8_t curve_;
  uint32_t last_tick_;
  int32_t apply_curve(int32_t v) const;
  bool change_led_;
};
}
