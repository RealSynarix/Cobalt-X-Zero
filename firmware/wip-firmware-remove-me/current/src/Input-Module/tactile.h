#pragma once
#include <stdint.h>
#include <stdbool.h>

namespace input {
struct ButtonState {
  bool pressed;
  uint32_t last_change_us;
  uint8_t debounce_ms;
  uint8_t stable_count;
};

class Tactile {
public:
  void init();
  void poll();
  bool is_pressed(uint8_t idx) const;
  uint8_t get_buttons() const;
  uint8_t get_debounce(uint8_t idx) const;
private:
  static constexpr uint8_t kNumButtons = 8;
  ButtonState states_[kNumButtons];
  uint8_t map_[kNumButtons];
  uint8_t raw_mask_;
  void read_hw();
  bool debounce_filter(ButtonState& s, bool raw, uint32_t now);
};
}
