#include "engine.h"
#define HYPRX_DIV_SHIFT 2
#define PENDING_MAX 512
static uint8_t mode=0, target=0, phase=0;
static uint8_t last_mac=0, last_mmb=0, last_lmb=0, last_rmb=0;
static int32_t pending=0, acc=0;
void hyprx_init(void){ mode=0; target=0; phase=0; last_mac=0; last_mmb=0; last_lmb=0; last_rmb=0; pending=0; acc=0; }
uint8_t hyprx_update(uint8_t buttons, int32_t *delta){
  uint8_t lmb = buttons & 1u;
  uint8_t rmb = (buttons >> 1) & 1u;
  uint8_t mmb = (buttons >> 2) & 1u;
  uint8_t mac = (buttons >> 3) & 1u;
  if(mac && !last_mac){
    mode ^= 1u;
    target = 0;
    pending=0; acc=0; phase=0;
  }
  if(mode){

    if(mmb){
      if(lmb && !last_lmb) target=0;
      if(rmb && !last_rmb) target=1;
    }

    if(mmb && !last_mmb){
      if(lmb && !rmb) target=0;
      else if(rmb && !lmb) target=1;
      else if(lmb && rmb) target=0;
    }
  }
  last_mac=mac; last_mmb=mmb; last_lmb=lmb; last_rmb=rmb;
  if(!mode || *delta==0){
    uint8_t base = buttons;
    if(mode) base &= ~0x04u;
    return base & 0x07u;
  }

  acc += *delta;
  int32_t s = acc >> HYPRX_DIV_SHIFT;
  acc -= s << HYPRX_DIV_SHIFT;
  if(s!=0){
    int32_t ad = s<0?-s:s;
    if(pending < PENDING_MAX) pending += ad;
  }
  *delta=0;
  return (buttons & ~0x04u) & 0x07u;
}
uint8_t hyprx_tick(void){
  if(!mode || pending<=0){ phase=0; return 0; }
  if(phase==0){ phase=1; return target==0?0x01u:0x02u; }
  phase=0; pending--; return 0;
}
uint8_t hyprx_active(void){ return mode; }
