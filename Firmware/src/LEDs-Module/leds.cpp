#include "leds.h"
#include "../HyprX-Module/macro.h"
#include <Arduino.h>
#include <stm32g4xx.h>
#include <math.h>

#define PI 3.1415926f
#define CYCLE_MS 10000u

static inline void set_rgb(uint8_t r,uint8_t g,uint8_t b){
  __disable_irq();
  TIM1->CCR1 = r;
  TIM1->CCR2 = g;
  TIM1->CCR3 = b;
  __enable_irq();
}

void leds_init(void){
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
  __DSB();
  GPIOA->MODER &= ~((3u<<16)|(3u<<18)|(3u<<20));
  GPIOA->MODER |= (2u<<16)|(2u<<18)|(2u<<20);
  GPIOA->OTYPER &= ~((1u<<8)|(1u<<9)|(1u<<10));
  GPIOA->OSPEEDR |= (3u<<16)|(3u<<18)|(3u<<20);
  GPIOA->PUPDR &= ~((3u<<16)|(3u<<18)|(3u<<20));
  GPIOA->AFR[1] &= ~((0xFu<<0)|(0xFu<<4)|(0xFu<<8));
  GPIOA->AFR[1] |= (1u<<0)|(1u<<4)|(1u<<8);
  TIM1->CR1 = 0;
  TIM1->PSC = 331;
  TIM1->ARR = 255;
  TIM1->CCMR1 = (6u<<4)|(1u<<3)|(6u<<12)|(1u<<11);
  TIM1->CCMR2 = (6u<<4)|(1u<<3);
  TIM1->CCER = TIM_CCER_CC1E|TIM_CCER_CC2E|TIM_CCER_CC3E;
  TIM1->BDTR = TIM_BDTR_MOE;
  TIM1->CCR1 = 0;
  TIM1->CCR2 = 0;
  TIM1->CCR3 = 0;
  TIM1->CR1 = TIM_CR1_ARPE|TIM_CR1_CEN;
}

void leds_tick(void){
  uint32_t t = millis() % CYCLE_MS;
  uint8_t Ra,Ga,Ba,Rb,Gb,Bb;
  if(macro_is_hyprx_active()){
    Ra = 255; Ga = 0; Ba = 0;
    Rb = 90; Gb = 0; Bb = 0;
  }else{
    Ra = 0; Ga = 0; Ba = 255;
    Rb = 0; Gb = 0; Bb = 90;
  }
  float r,g,b;
  if(t < 3000u){
    r = Ra; g = Ga; b = Ba;
  }else if(t < 4000u){
    float p = (float)(t-3000u)/1000.0f;
    float e = 0.5f - 0.5f * cosf(p*PI);
    r = Ra*(1.0f-e) + Rb*e;
    g = Ga*(1.0f-e) + Gb*e;
    b = Ba*(1.0f-e) + Bb*e;
  }else if(t < 8000u){
    r = Rb; g = Gb; b = Bb;
  }else if(t < 9000u){
    float p = (float)(t-8000u)/1000.0f;
    float e = 0.5f - 0.5f * cosf(p*PI);
    r = Rb*(1.0f-e) + Ra*e;
    g = Gb*(1.0f-e) + Ga*e;
    b = Bb*(1.0f-e) + Ba*e;
  }else{
    r = Ra; g = Ga; b = Ba;
  }
  if(r<2) r=0;
  if(g<2) g=0;
  if(b<2) b=0;
  if(r>253) r=255;
  if(g>253) g=255;
  if(b>253) b=255;
  set_rgb((uint8_t)r,(uint8_t)g,(uint8_t)b);
}
