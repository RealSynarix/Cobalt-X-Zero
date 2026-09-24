#include "indicator.h"
#include "src/Config-Module/config.h"
#include <Arduino.h>

namespace lights {
void Indicator::init() {
  const auto* c = cfg::cfg_get();
  base_color_ = c->led_default;
  macro_color_ = c->led_macro;
  brightness_ = c->led_brightness;
  effect_ = (Effect)c->led_effect;
  breath_speed_ = c->led_breathing_speed;
  reactive_trig_ = false;
  reactive_start_ = 0;
  off_on_motion_ = c->led_off_on_motion;
  last_motion_ = millis();
  phase_ = 0;
}

uint32_t Indicator::apply_brightness(uint32_t c) const {
  uint8_t r = (c >> 16) & 0xFF;
  uint8_t g = (c >> 8) & 0xFF;
  uint8_t b = c & 0xFF;
  r = (r * brightness_) >> 8;
  g = (g * brightness_) >> 8;
  b = (b * brightness_) >> 8;
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void Indicator::set_color(uint32_t rgb) { base_color_ = rgb & 0xFFFFFF; }
void Indicator::set_macro_color(uint32_t rgb) { macro_color_ = rgb & 0xFFFFFF; }
void Indicator::set_brightness(uint8_t b) { brightness_ = b; }
void Indicator::set_effect(Effect e) { effect_ = e; }
void Indicator::set_breathing_speed(uint8_t s) { if (s) breath_speed_ = s; }
void Indicator::trigger_reactive() { reactive_trig_ = true; reactive_start_ = millis(); }
void Indicator::set_off_on_motion(bool v) { off_on_motion_ = v; }

void Indicator::update() {
  uint32_t now = millis();
  phase_ += breath_speed_;
  uint32_t out = base_color_;
  switch (effect_) {
    case Effect::Solid: out = base_color_; break;
    case Effect::Breathing: {
      uint8_t k = (uint8_t)((phase_ >> 8) & 0xFF);
      uint8_t factor = k < 128 ? k * 2 : (255 - k) * 2;
      out = apply_brightness(base_color_);
      uint8_t r = ((out >> 16) & 0xFF) * factor >> 8;
      uint8_t g = ((out >> 8) & 0xFF) * factor >> 8;
      uint8_t b = (out & 0xFF) * factor >> 8;
      out = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
      break;
    }
    case Effect::Reactive: {
      if (reactive_trig_ && (now - reactive_start_) < 300) out = macro_color_;
      else { reactive_trig_ = false; out = base_color_; }
      out = apply_brightness(out);
      break;
    }
    case Effect::Rainbow: {
      uint8_t h = (phase_ >> 10) & 0xFF;
      out = apply_brightness((uint32_t)h << 16 | (uint32_t)(255 - h) << 8 | (uint32_t)(h >> 1));
      break;
    }
    case Effect::Off: out = 0; break;
  }
  if (off_on_motion_) {
    const auto* c = cfg::cfg_get();
    if (c->led_off_timeout && (now - last_motion_) > c->led_off_timeout) out = 0;
  }
  (void)out;
}
}
