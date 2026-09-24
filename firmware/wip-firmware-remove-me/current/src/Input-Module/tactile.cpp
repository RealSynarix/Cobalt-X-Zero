#include "tactile.h"
#include "src/Config-Module/config.h"
#include <Arduino.h>

namespace input {
void Tactile::init() {
  for (uint8_t i = 0; i < kNumButtons; ++i) {
    states_[i] = {false, 0, 4, 0};
    map_[i] = i;
  }
  raw_mask_ = 0;
  const auto* c = cfg::cfg_get();
  states_[0].debounce_ms = c->debounce_pb3;
  states_[1].debounce_ms = c->debounce_pb4;
  states_[2].debounce_ms = c->debounce_pb5;
  states_[3].debounce_ms = c->debounce_pb6;
  states_[4].debounce_ms = c->debounce_pb7;
}

void Tactile::read_hw() {
  raw_mask_ = 0;
}

bool Tactile::debounce_filter(ButtonState& s, bool raw, uint32_t now) {
  if (raw != s.pressed) {
    if ((now - s.last_change_us) >= (uint32_t)s.debounce_ms * 1000UL) {
      s.pressed = raw;
      s.last_change_us = now;
      s.stable_count = 0;
      return true;
    }
  } else {
    s.stable_count++;
  }
  return false;
}

void Tactile::poll() {
  uint32_t now = micros();
  read_hw();
  for (uint8_t i = 0; i < kNumButtons; ++i) {
    bool raw = (raw_mask_ >> i) & 1;
    debounce_filter(states_[i], raw, now);
  }
}

bool Tactile::is_pressed(uint8_t idx) const {
  if (idx >= kNumButtons) return false;
  return states_[idx].pressed;
}

uint8_t Tactile::get_buttons() const {
  uint8_t m = 0;
  for (uint8_t i = 0; i < kNumButtons; ++i) if (states_[i].pressed) m |= (1 << i);
  return m;
}

uint8_t Tactile::get_debounce(uint8_t idx) const {
  if (idx >= kNumButtons) return 0;
  return states_[idx].debounce_ms;
}
}
