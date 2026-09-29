#include "engine.h"
#include "../Config-Module/config.h"
#include <stm32g4xx.h>

static uint8_t mode=0,target=0;
static uint8_t last_mac=0,last_mmb=0,last_lmb=0,last_rmb=0;
static int32_t pending=0,acc=0;
static uint32_t stamp=0;

static void clear(void){ pending=0; acc=0; stamp=0; }

void hyprx_init(void){ mode=0; target=0; last_mac=0; last_mmb=0; last_lmb=0; last_rmb=0; stamp=0; clear(); }

uint8_t hyprx_update(uint8_t b, int32_t *d){
  uint8_t lmb = b & 1u;
  uint8_t rmb = (b >> 1) & 1u;
  uint8_t mmb = (b >> 2) & 1u;
  uint8_t mac = (b >> 3) & 1u;
  uint8_t armed = g_cfg.mod_hyprx && g_cfg.hyprx_en;

  if(!armed && mode){ mode=0; clear(); }

  if(armed && mac && !last_mac){
    mode ^= 1u;
    target = g_cfg.hyprx_target; // default LMB
    clear();
  }

  if(mode){
    // Swap target while holding MMB
    if(mmb){
      if(lmb && !last_lmb) target = 0; // LMB
      if(rmb && !last_rmb) target = 1; // RMB
    }
    if(mmb && !last_mmb){
      if(lmb && !rmb) target = 0;
      else if(rmb && !lmb) target = 1;
      else if(lmb && rmb) target = 0; // both = reset to LMB
    }
  }

  last_mac = mac;
  last_mmb = mmb;
  last_lmb = lmb;
  last_rmb = rmb;

  if(!mode){
    uint8_t base = b;
    base &= ~0x04u; // clear MMB from report when not needed? Keep original behavior
    return base & 0x0Fu;
  }

  // HyprX active: handle scroll -> click
  int32_t delta = d ? *d : 0;
  if(delta != 0){
    // accumulate, respect hyprx_div as divisor for sensitivity
    // Original bug: dv = delta / div truncates 1/4 to 0. Fix: accumulate delta, threshold = stc * div
    // For each scroll = click, set div=1, stc=1
    int32_t div = g_cfg.hyprx_div ? g_cfg.hyprx_div : 1;
    int32_t st = g_cfg.hyprx_stc ? g_cfg.hyprx_stc : 1;
    // To make div work as "scrolls per click", threshold = st * div
    // But we want each scroll = click by default, so config defaults div=1, st=1
    pending += delta;
    int32_t thresh = st * div;
    if(thresh < 1) thresh = 1;
    while(pending >= thresh || pending <= -thresh){
      acc += (pending > 0) ? 1 : -1;
      pending -= (pending > 0) ? thresh : -thresh;
      stamp = DWT->CYCCNT;
    }
  }

  if(acc != 0){
    uint32_t now = DWT->CYCCNT;
    uint32_t hold = g_cfg.hyprx_hold * (SystemCoreClock / 1000u);
    if((now - stamp) >= hold){
      int32_t out = (acc > 0) ? 1 : -1;
      acc -= out;
      if(d) *d = out;
      uint8_t base = b;
      base &= ~0x04u; // don't send MMB while spamming
      if(target == 0) base |= 0x01u;
      else base |= 0x02u;
      return base & 0x0Fu;
    }
  }

  if(d) *d = 0;
  uint8_t base = b;
  if(mode) base &= ~0x04u;
  return base & 0x0Fu;
}

uint8_t hyprx_tick(void){ return mode; }
uint8_t hyprx_active(void){ return mode; }
