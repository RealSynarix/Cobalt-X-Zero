#include <cstdint>
#include "macro.h"

#define HYPRX_DIV 4
static uint8_t hyprx_mode=0, active_target=0, press_phase=0, last_mac=0, last_mmb=0;
static int32_t pending=0, acc=0;
static uint32_t mmb_hold=0;

void macro_init(void){
  hyprx_mode=0;
  active_target=0;
  pending=0;
  acc=0;
  press_phase=0;
  last_mac=0;
  last_mmb=0;
  mmb_hold=0;
}

uint8_t macro_update(uint8_t buttons,int32_t* delta){
  uint8_t mac=(buttons>>3)&1u;
  if(mac &&!last_mac){
    hyprx_mode^=1u;
    active_target=0;
    pending=0;
    acc=0;
    press_phase=0;
  }
  last_mac=mac;
  uint8_t mmb=(buttons>>2)&1u;
  int32_t d=*delta;
  if(hyprx_mode){
    if(d!=0){
      mmb_hold=0;
    }else{
      if(mmb){
        mmb_hold++;
        if(mmb_hold>1500){
          if(!last_mmb) active_target^=1u;
          mmb_hold=0;
          last_mmb=1;
        }
      }else{
        mmb_hold=0;
        last_mmb=0;
      }
    }
  }else{
    last_mmb=mmb;
  }
  if(hyprx_mode && d!=0){
    acc+=d;
    int32_t s=acc/HYPRX_DIV;
    acc-=s*HYPRX_DIV;
    if(s!=0){
      int32_t ad=s<0?-s:s;
      if(pending<512) pending+=ad;
    }
    *delta=0;
  }
  uint8_t base=buttons;
  if(hyprx_mode) base&=~0x04u;
  return base & 0x07u;
}

uint8_t macro_frame_tick(void){
  if(!hyprx_mode || pending<=0){ press_phase=0; return 0; }
  if(press_phase==0){ press_phase=1; return active_target==0?0x01u:0x02u; }
  else{ press_phase=0; pending--; return 0; }
}

uint8_t macro_is_hyprx_active(void){ return hyprx_mode; }
