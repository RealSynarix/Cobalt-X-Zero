#include "tactile.h"
#include "../HyprX-Module/engine.h"
#include "../Config-Module/config.h"
#include <stm32g4xx.h>

#define BASE_LOCK 34000u
#define FRAME_DIV 8
#define QSIZE 64
#define QMASK 63

typedef struct { uint8_t b; int8_t w; uint8_t m; } qitem_t;

static qitem_t q[QSIZE];
static volatile uint8_t qh=0, qt=0, qc=0;
static volatile uint8_t g_buttons=0;
static uint8_t phys_state=0;
static uint32_t last_cyc[5]={0,0,0,0,0};
static int32_t wheel_acc=0;
static uint32_t last_enc=0;
static uint8_t frame=0;
static volatile uint8_t macro_q=0;
static uint8_t side_state=0;

static inline void qpush(uint8_t b, int8_t w, uint8_t m){
  uint32_t p=__get_PRIMASK(); __disable_irq();
  if(qc<QSIZE){ q[qh].b=b; q[qh].w=w; q[qh].m=m; qh=(qh+1)&QMASK; qc++; }
  if(!p) __enable_irq();
}

static inline uint32_t lockout(uint8_t i){
  uint32_t ms;
  switch(i){ case 0: ms=g_cfg.db_pb3; break; case 1: ms=g_cfg.db_pb4; break; case 2: ms=g_cfg.db_pb5; break; case 3: ms=g_cfg.db_pb6; break; default: ms=g_cfg.db_pb7; break; }
  ms*=SystemCoreClock/1000u;
  switch(g_cfg.click_mode){ case 1: return ms*2u; case 2: return ms/2u; default: return ms; }
}

static inline uint8_t read_pins(void){
  uint32_t idr=GPIOB->IDR;
  uint8_t s=0;
  if(!(idr&(1u<<3))) s|=1u<<0;
  if(!(idr&(1u<<4))) s|=1u<<1;
  if(!(idr&(1u<<5))) s|=1u<<2;
  if(!(idr&(1u<<6))) s|=1u<<3;
  if(!(idr&(1u<<7))) s|=1u<<4;
  return s;
}

static inline int8_t read_wheel(void){
  uint32_t a=(GPIOA->IDR>>0)&1u;
  uint32_t b=(GPIOA->IDR>>1)&1u;
  uint32_t cur=(a<<1)|b;
  int8_t dir=0;
  if(last_enc==0){ if(cur==1) dir=1; else if(cur==2) dir=-1; }
  else if(last_enc==1){ if(cur==3) dir=1; else if(cur==0) dir=-1; }
  else if(last_enc==3){ if(cur==2) dir=1; else if(cur==1) dir=-1; }
  else if(last_enc==2){ if(cur==0) dir=1; else if(cur==3) dir=-1; }
  last_enc=cur;
  return dir;
}

extern "C"{

void tactile_init(void){
  RCC->AHB2ENR|=RCC_AHB2ENR_GPIOBEN|RCC_AHB2ENR_GPIOAEN; __DSB();
  GPIOB->MODER&=~((0x3u<<6)|(0x3u<<8)|(0x3u<<10)|(0x3u<<12)|(0x3u<<14));
  GPIOB->PUPDR=(GPIOB->PUPDR&~((0x3u<<6)|(0x3u<<8)|(0x3u<<10)|(0x3u<<12)|(0x3u<<14)))|((1u<<6)|(1u<<8)|(1u<<10)|(1u<<12)|(1u<<14));
  GPIOA->MODER&=~((0x3u<<0)|(0x3u<<2));
  GPIOA->PUPDR=(GPIOA->PUPDR&~((0x3u<<0)|(0x3u<<2)))|((1u<<0)|(1u<<2));
  GPIOB->IDR; GPIOA->IDR;
  last_enc=(GPIOA->IDR&0x3u);
  phys_state=read_pins();
  g_buttons=0; side_state=0; qh=qt=qc=0; wheel_acc=0; macro_q=0; frame=0;
  hyprx_init();
}

void tactile_tick(void){
  frame++;
  uint8_t now=read_pins();
  uint32_t cyc=DWT->CYCCNT;
  for(uint8_t i=0;i<5;i++){
    uint8_t mask=1u<<i;
    uint8_t cur=now&mask;
    uint8_t prev=phys_state&mask;
    if(cur!=prev){
      if((cyc-last_cyc[i])>=lockout(i)){
        phys_state=(phys_state&~mask)|cur;
        last_cyc[i]=cyc;
        if(cur){ if(i<3) g_buttons|=mask; else if(i==3) side_state=1; else macro_q=1; }
        else{ if(i<3) g_buttons&=(uint8_t)~mask; else if(i==3) side_state=0; }
      }
    }
  }

  // Always update HyprX for toggle and target swap (MMB+LMB etc)
  uint8_t b_for = (g_buttons&0x07) | (side_state?0x08:0x00);
  int32_t d0=0;
  hyprx_update(b_for, &d0);

  int8_t wd = read_wheel();
  if(wd){
    if(hyprx_active()){
      int32_t d = wd;
      uint8_t b_res = hyprx_update(b_for, &d);
      if(d!=0){
        uint8_t b_final = b_res & 0x0F;
        b_final &= ~0x08;
        // Push press
        qpush(b_final, 0, 0);
        // Push release quickly after to create rapid click
        // The next periodic push will also release, but push explicit release for reliability
        qpush(b_final & ~0x03, 0, 0);
      }
    }else{
      wheel_acc+=wd;
      int32_t div=g_cfg.wheel_div?g_cfg.wheel_div:3;
      while(wheel_acc>=div || wheel_acc<=-div){
        int8_t w=wheel_acc>0?1:-1;
        if(g_cfg.wheel_inv) w=-w;
        qpush(g_buttons&0x07, w, 0);
        wheel_acc-=(wheel_acc>0?div:-div);
      }
    }
  }

  if((frame%FRAME_DIV)==0){
    b_for = (g_buttons&0x07) | (side_state?0x08:0x00);
    d0=0;
    uint8_t b_after = hyprx_update(b_for, &d0);
    uint8_t b_final = b_after & 0x0F;
    b_final &= ~0x08;
    // If HyprX active and has pending clicks but no new wheel, try to flush with hold timing
    if(hyprx_active() && d0==0){
      // Call again with d=0 to allow hold timer to fire? Engine now handles acc even when d=0 via first part? Actually it needs d !=0 to trigger, but we already handle acc in the d!=0 path
      // To flush pending clicks from previous scrolls with hold>0, we can call with d=0 and check if it would output
      int32_t d_flush=0;
      uint8_t b_flush = hyprx_update(b_for, &d_flush);
      if(d_flush!=0){
        b_final = b_flush & 0x0F;
        b_final &= ~0x08;
        qpush(b_final, 0, 0);
        qpush(b_final & ~0x03, 0, 0);
      }else{
        qpush(b_final & 0x07, 0, macro_q);
        macro_q=0;
      }
    }else{
      qpush(b_final & 0x07, 0, macro_q);
      macro_q=0;
    }
  }
}

uint8_t tactile_pop(tactile_report_t *out){
  uint32_t p=__get_PRIMASK(); __disable_irq();
  if(qc==0){ if(!p) __enable_irq(); return 0; }
  qitem_t it=q[qt]; qt=(qt+1)&QMASK; qc--; if(!p) __enable_irq();
  out->buttons=it.b; out->wheel=it.w; out->macro=it.m; return 1;
}

}
