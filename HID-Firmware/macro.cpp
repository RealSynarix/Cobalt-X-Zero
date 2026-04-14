#include "macro.h"

static uint8_t hyprx_mode = 0;
static uint8_t active_target = 0;
static int32_t pending = 0;
static uint8_t state = 0;
static uint8_t last_mac = 0;
static uint8_t last_mmb = 0;

void macro_init(void) {
  hyprx_mode = 0;
  active_target = 0;
  pending = 0;
  state = 0;
  last_mac = 0;
  last_mmb = 0;
}

uint8_t macro_update(uint8_t buttons, int32_t *delta) {
  uint8_t mac = (buttons >> 3) & 1;
  if (mac && !last_mac) {
    hyprx_mode = !hyprx_mode;
    if (!hyprx_mode) {
      pending = 0;
      state = 0;
    }
  }
  last_mac = mac;

  uint8_t mmb = (buttons >> 2) & 1;
  if (hyprx_mode && mmb && !last_mmb) {
    active_target = !active_target;
  }
  last_mmb = mmb;

  if (hyprx_mode) {
    int32_t d = *delta;
    if (d != 0) {
      pending += (d > 0) ? (d * 2) : (-d * 2);
      *delta = 0;
    }
  }

  uint8_t synth = 0;
  if (hyprx_mode && pending > 0) {
    if (state) {
      state = 0;
      pending--;
    } else {
      state = 1;
    }
    if (state) synth = (active_target == 0) ? 0x01 : 0x02;
  } else {
    state = 0;
  }

  uint8_t final = buttons;
  if (hyprx_mode) {
    final &= ~0x04;
    final |= synth;
  }
  return final & 0x07;
}