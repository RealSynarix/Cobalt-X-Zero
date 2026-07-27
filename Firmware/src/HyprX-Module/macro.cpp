#include "macro.h"

#define HYPRX_DIV 2

static uint8_t hyprx_mode = 0;
static uint8_t active_target = 0;
static int32_t pending = 0;
static int32_t acc = 0;
static uint8_t press_phase = 0;
static uint8_t last_mac = 0;
static uint8_t last_mmb = 0;

void macro_init(void) {
  hyprx_mode = 0;
  active_target = 0;
  pending = 0;
  acc = 0;
  press_phase = 0;
  last_mac = 0;
  last_mmb = 0;
}

uint8_t macro_update(uint8_t buttons, int32_t *delta) {
  uint8_t mac = (buttons >> 3) & 1u;
  if (mac &&!last_mac) {
    hyprx_mode ^= 1u;
    if (hyprx_mode) {
      active_target = 0;
      pending = 0;
      acc = 0;
      press_phase = 0;
    } else {
      pending = 0;
      acc = 0;
      press_phase = 0;
    }
  }
  last_mac = mac;
  uint8_t mmb = (buttons >> 2) & 1u;
  if (hyprx_mode && mmb &&!last_mmb) {
    active_target ^= 1u;
  }
  last_mmb = mmb;
  if (hyprx_mode) {
    if (*delta!= 0) {
      acc += *delta;
      int32_t s = acc / HYPRX_DIV;
      acc %= HYPRX_DIV;
      if (s!= 0) {
        int32_t ad = s < 0? -s : s;
        if (pending < 512) pending += ad;
      }
      *delta = 0;
    }
  }
  uint8_t base = buttons;
  if (hyprx_mode) base &= (uint8_t)~0x04u;
  return base & 0x07u;
}

uint8_t macro_frame_tick(void) {
  if (!hyprx_mode || pending <= 0) {
    press_phase = 0;
    return 0;
  }
  if (press_phase == 0) {
    press_phase = 1;
    return active_target == 0? 0x01u : 0x02u;
  } else {
    press_phase = 0;
    pending--;
    return 0;
  }
}

uint8_t macro_is_hyprx_active(void) {
  return hyprx_mode;
}
