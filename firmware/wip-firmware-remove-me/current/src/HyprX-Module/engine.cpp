#include "engine.h"
#include "src/Config-Module/config.h"
#include "src/Lights-Module/indicator.h"
#include <Arduino.h>

namespace hyprx {
void Engine::init() {
  const auto* c = cfg::cfg_get();
  enabled_ = c->hyprx_enable;
  state_.accum = 0;
  state_.div_shift = c->hyprx_div_shift;
  state_.hold_ticks = 0;
  state_.pending = 0;
  state_.target = c->hyprx_default_target;
  pending_max_ = c->hyprx_pending_max;
  scroll_to_click_ = c->hyprx_scroll_to_click;
  activation_ = c->hyprx_activation;
  curve_ = c->hyprx_curve;
  change_led_ = c->hyprx_change_led;
  last_tick_ = micros();
}

void Engine::set_enabled(bool en) { enabled_ = en; }
bool Engine::is_enabled() const { return enabled_; }
uint8_t Engine::get_pending() const { return state_.pending; }
void Engine::reset() { state_.accum = 0; state_.pending = 0; state_.hold_ticks = 0; }

int32_t Engine::apply_curve(int32_t v) const {
  switch (curve_) {
    case 0: return v;
    case 1: return v + (v >> 1);
    case 2: return v * 2;
    case 3: return (v * v) >> 4;
    case 4: return v + ((v * v) >> 6);
    default: return v;
  }
}

void Engine::feed_scroll(int8_t delta) {
  if (!enabled_) return;
  int32_t curved = apply_curve(delta);
  state_.accum += curved;
  int32_t thr = 1 << state_.div_shift;
  while (state_.accum >= thr && state_.pending < pending_max_) {
    state_.accum -= thr;
    state_.pending++;
    state_.hold_ticks = cfg::cfg_get()->hyprx_hold_ticks;
  }
  while (state_.accum <= -thr && state_.pending < pending_max_) {
    state_.accum += thr;
    state_.pending++;
    state_.hold_ticks = cfg::cfg_get()->hyprx_hold_ticks;
  }
}

void Engine::update() {
  if (!enabled_) return;
  uint32_t now = micros();
  uint32_t dt = now - last_tick_;
  last_tick_ = now;
  if (state_.pending > 0) {
    if (state_.hold_ticks > 0) {
      if (dt >= 1000) state_.hold_ticks--;
    } else {
      if (scroll_to_click_) {
        state_.pending--;
        state_.hold_ticks = cfg::cfg_get()->hyprx_hold_ticks;
      }
    }
  }
}
}
