#include "tactile.h"
#include "../HyprX-Module/engine.h"
#include <stm32g4xx.h>
#define ENC_DIV 3
#define FRAME_DIV 32
#define QSIZE 16
#define QMASK 15
typedef struct { uint8_t buttons; int8_t wheel; } qitem_t;
static qitem_t q[QSIZE];
static volatile uint8_t qh=0, qt=0, qc=0;
static volatile uint8_t g_buttons=0;
static uint8_t phys_state=0;
static uint32_t last_cyc[4]={0,0,0,0};
static int32_t wheel_acc=0;
static uint32_t last_enc=0;
static uint8_t frame=0;
static inline void qpush(uint8_t b, int8_t w){
  uint32_t p=__get_PRIMASK(); __disable_irq();
  if(qc < QSIZE){ q[qh].buttons=b; q[qh].wheel=w; qh=(qh+1)&QMASK; qc++; }
  if(!p) __enable_irq();
}
void tactile_init(void){
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN; __DSB();
  GPIOB->MODER &= ~(0xFFu << 6u);
  GPIOB->PUPDR = (GPIOB->PUPDR & ~(0xFFu << 6u)) | (0x55u << 6u);
  GPIOA->MODER = (GPIOA->MODER & ~0x0Fu) | 0x0Au;
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~0xFFu) | 0x11u;
  GPIOA->PUPDR = (GPIOA->PUPDR & ~0x0Fu) | 0x05u;
  GPIOA->OSPEEDR = (GPIOA->OSPEEDR & ~0x0Fu) | 0x0Fu;
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; __DSB();
  TIM2->CR1=0; TIM2->SMCR=3; TIM2->CCMR1=0x6161u;
  TIM2->CCER=TIM_CCER_CC1E|TIM_CCER_CC2E;
  TIM2->ARR=0xFFFFFFFFu; TIM2->CNT=0x80000000u; TIM2->CR1=TIM_CR1_CEN;
  last_enc=TIM2->CNT; wheel_acc=0; phys_state=0; g_buttons=0; qh=qt=qc=0; frame=0;
  hyprx_init();
}
void tactile_tick(void){
  uint32_t now = DWT->CYCCNT;
  uint32_t idr = GPIOB->IDR;
  for(uint8_t i=0;i<4;i++){
    uint8_t pin=3+i;
    uint8_t phys = ((idr>>pin)&1u)==0u;
    uint8_t curr = (phys_state>>i)&1u;
    if(phys==curr) continue;
    if(phys){
      phys_state |= (1u<<i);
      g_buttons = phys_state & 0x07u;
      last_cyc[i]=now;
      int32_t d=0; uint8_t base = hyprx_update(phys_state,&d);
      uint8_t synth = hyprx_tick();
      qpush((base|synth)&0x07u,0);
    }else{
      if((now-last_cyc[i]) < 35000u) continue;
      phys_state &= ~(1u<<i);
      g_buttons = phys_state & 0x07u;
      last_cyc[i]=now;
      int32_t d=0; uint8_t base = hyprx_update(phys_state,&d);
      uint8_t synth = hyprx_tick();
      qpush((base|synth)&0x07u,0);
    }
  }
  uint32_t enc = TIM2->CNT;
  int32_t delta = (int32_t)(enc - last_enc);
  last_enc = enc;
  if(delta==0) goto frame_check;
  if(delta>1000 || delta<-1000){ wheel_acc=0; goto frame_check; }
  {
    int32_t d = delta;
    uint8_t base = hyprx_update(phys_state,&d);
    if(d==0){
      wheel_acc=0;
    }else{
      wheel_acc += d;
      int32_t steps = wheel_acc / ENC_DIV;
      if(steps!=0){
        wheel_acc -= steps*ENC_DIV;
        if(steps>127) steps=127; if(steps<-127) steps=-127;
        uint8_t synth = hyprx_tick();
        qpush((base|synth)&0x07u,(int8_t)steps);
      }
    }
  }
frame_check:
  frame++;
  if(frame < FRAME_DIV) return;
  frame=0;
  int32_t d=0; uint8_t base = hyprx_update(phys_state,&d);
  uint8_t synth = hyprx_tick();
  if(synth) qpush((base|synth)&0x07u,0);
}
uint8_t tactile_pop(tactile_report_t *out){
  if(!out) return 0;
  uint32_t p=__get_PRIMASK(); __disable_irq();
  if(qc){
    out->buttons = q[qt].buttons;
    out->wheel = q[qt].wheel;
    qt=(qt+1)&QMASK; qc--;
    if(!p) __enable_irq();
    return 1;
  }
  if(!p) __enable_irq();
  out->buttons = g_buttons;
  out->wheel = 0;
  return 1;
}
