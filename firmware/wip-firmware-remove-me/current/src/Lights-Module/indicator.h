#pragma once
#include <stdint.h>
#include <stdbool.h>

namespace lights {
enum class Effect : uint8_t {
  Solid = 0,
  Breathing,
  Reactive,
  Rainbow,
  Off
};

class Indicator {
public:
  void init();
  void update();
  void set_color(uint32_t rgb);
  void set_macro_color(uint32_t rgb);
  void set_brightness(uint8_t b);
  void set_effect(Effect e);
  void set_breathing_speed(uint8_t s);
  void trigger_reactive();
  void set_off_on_motion(bool v);
private:
  uint32_t base_color_;
  uint32_t macro_color_;
  uint8_t brightness_;
  Effect effect_;
  uint8_t breath_speed_;
  bool reactive_trig_;
  uint32_t reactive_start_;
  bool off_on_motion_;
  uint32_t last_motion_;
  uint32_t phase_;
  uint32_t apply_brightness(uint32_t c) const;
};
}
