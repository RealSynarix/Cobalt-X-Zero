#include "indicator.h"
#include "../HyprX-Module/engine.h"
#include <stm32g4xx.h>
#include <Arduino.h>
#define CYCLE_MS 15000u
static void tim1_pwm_init(void){
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; __DSB();
  GPIOA->MODER = (GPIOA->MODER & ~(0x3Fu << 16u)) | (0x2Au << 16u);
  GPIOA->AFR[1] = (GPIOA->AFR[1] & ~(0xFFFu << 0u)) | (0x666u << 0u);
  GPIOA->OSPEEDR |= (0x3Fu << 16u);
  GPIOA->PUPDR &= ~(0x3Fu << 16u);
  TIM1->PSC = 0; TIM1->ARR = 255;
  TIM1->CCMR1 = (6u<<4u) | (6u<<12u) | TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
  TIM1->CCMR2 = (6u<<4u) | TIM_CCMR2_OC3PE;
  TIM1->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;
  TIM1->BDTR = TIM_BDTR_MOE | TIM_BDTR_OSSR | TIM_BDTR_OSSI;
  TIM1->CCR1=0; TIM1->CCR2=0; TIM1->CCR3=0;
  TIM1->CR1 = TIM_CR1_ARPE | TIM_CR1_CEN;
}
static inline void set_rgb_hw(uint8_t r, uint8_t g, uint8_t b){ TIM1->CCR1=r; TIM1->CCR2=g; TIM1->CCR3=b; }
void indicator_init(void){ tim1_pwm_init(); }
void indicator_tick(void){
  uint32_t t = millis() % CYCLE_MS;
  uint32_t phase = (t * 256u) / CYCLE_MS;
  uint32_t tri = (phase < 128u) ? (phase*2u) : ((255u-phase)*2u);
  uint32_t x = tri;
  uint32_t smooth = (x * x * (768u - (x<<1))) >> 16;
  uint32_t f = 5u + (smooth * 250u >> 8);
  uint8_t v = (uint8_t)f;
  if(hyprx_active()) set_rgb_hw(v,v,0); else set_rgb_hw(0,0,v);
}
