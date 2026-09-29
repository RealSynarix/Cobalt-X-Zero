#include "indicator.h"
#include "../HyprX-Module/engine.h"
#include "../Config-Module/config.h"
#include "../USB-Module/hid.h"
#include <stm32g4xx.h>
#include <Arduino.h>
static void tim1_pwm_init(void){
RCC->APB2ENR|=RCC_APB2ENR_TIM1EN;RCC->AHB2ENR|=RCC_AHB2ENR_GPIOAEN;__DSB();
GPIOA->MODER=(GPIOA->MODER&~(0x3Fu<<16u))|(0x2Au<<16u);GPIOA->AFR[1]=(GPIOA->AFR[1]&~(0xFFFu<<0u))|(0x666u<<0u);GPIOA->OSPEEDR|=(0x3Fu<<16u);GPIOA->PUPDR&=~(0x3Fu<<16u);
TIM1->PSC=0;TIM1->ARR=255;TIM1->CCMR1=(6u<<4u)|(6u<<12u)|TIM_CCMR1_OC1PE|TIM_CCMR1_OC2PE;TIM1->CCMR2=(6u<<4u)|TIM_CCMR2_OC3PE;TIM1->CCER=TIM_CCER_CC1E|TIM_CCER_CC2E|TIM_CCER_CC3E;TIM1->BDTR=TIM_BDTR_MOE|TIM_BDTR_OSSR|TIM_BDTR_OSSI;TIM1->CCR1=TIM1->CCR2=TIM1->CCR3=0;TIM1->CR1=TIM_CR1_ARPE|TIM_CR1_CEN;
}
static inline void set_rgb(uint8_t r,uint8_t g,uint8_t b){TIM1->CCR1=r;TIM1->CCR2=g;TIM1->CCR3=b;}
static uint32_t level(void){
uint32_t speed=g_cfg.led_speed<10u?10u:g_cfg.led_speed;uint32_t cycle=1500000u/speed;uint32_t ph=((millis()%cycle)*256u)/cycle;
switch(g_cfg.led_eff){
case 0:return 255;
case 1:{uint32_t tri=ph<128u?ph*2u:(255u-ph)*2u;uint32_t s=(tri*tri*(768u-(tri<<1)))>>16;return 5u+(s*250u>>8);}
case 2:return(ph<128u)?255:0;
case 3:return 255;
case 4:return(ph<128u)?255:180;
default:return 255;
}}
extern "C"{
void indicator_init(void){tim1_pwm_init();}
void indicator_tick(void){
uint32_t now=millis();
if(g_cfg.led_idle&&now-hid_last_activity_ms()>=((uint32_t)g_cfg.led_idle*1000u)){set_rgb(0,0,0);return;}
uint8_t r=g_cfg.led_def[0],g=g_cfg.led_def[1],b=g_cfg.led_def[2];
if(g_cfg.hyprx_en&&g_cfg.hyprx_led&&hyprx_active()){r=g_cfg.led_macro[0];g=g_cfg.led_macro[1];b=g_cfg.led_macro[2];}
else if(g_cfg.led_activity&&now-hid_last_activity_ms()<100u){r=g_cfg.led_macro[0];g=g_cfg.led_macro[1];b=g_cfg.led_macro[2];}
uint32_t lvl=level(),bri=g_cfg.led_bri;
if(g_cfg.led_eff==3){
uint32_t cycle=10000u/((g_cfg.led_speed/25u)+1);if(cycle<500)cycle=500;uint8_t pos=(millis()*255/cycle)%255;uint8_t rr,gg,bb;
if(pos<85){rr=255-pos*3;gg=pos*3;bb=0;}else if(pos<170){pos-=85;rr=0;gg=255-pos*3;bb=pos*3;}else{pos-=170;rr=pos*3;gg=0;bb=255-pos*3;}
set_rgb((rr*bri)/255,(gg*bri)/255,(bb*bri)/255);return;
}
set_rgb((uint8_t)(((uint32_t)r*lvl*bri)/65025u),(uint8_t)(((uint32_t)g*lvl*bri)/65025u),(uint8_t)(((uint32_t)b*lvl*bri)/65025u));
}
}
