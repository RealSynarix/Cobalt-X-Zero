#include "leds.h"
#include "../HyprX-Module/macro.h"
#include <Arduino.h>
#include <math.h>

#define PI 3.1415926f
#define CYCLE_MS 10000u

static inline void set_rgb(uint8_t r,uint8_t g,uint8_t b){
  if(r==0){ pinMode(PA8,OUTPUT); digitalWrite(PA8,LOW); } else analogWrite(PA8,r);
  if(g==0){ pinMode(PA9,OUTPUT); digitalWrite(PA9,LOW); } else analogWrite(PA9,g);
  if(b==0){ pinMode(PA10,OUTPUT); digitalWrite(PA10,LOW); } else analogWrite(PA10,b);
}

void leds_init(void){
  analogWriteResolution(8);
  pinMode(PA8,OUTPUT); pinMode(PA9,OUTPUT); pinMode(PA10,OUTPUT);
  digitalWrite(PA8,LOW); digitalWrite(PA9,LOW); digitalWrite(PA10,LOW);
}

void leds_tick(void){
  uint32_t t = millis() % CYCLE_MS;
  float f = 0.5f - 0.5f * cosf((float)t / CYCLE_MS * 2.0f * PI);
  if(macro_is_hyprx_active()){
    float mix = (t < 5000u)? (float)t/5000.0f : (float)(10000u - t)/5000.0f;
    uint8_t cr=192, cg=0, cb=10;
    uint8_t or_r=255, or_g=90, or_b=0;
    float mr = cr*(1.0f-mix) + or_r*mix;
    float mg = cg*(1.0f-mix) + or_g*mix;
    float mb = cb*(1.0f-mix) + or_b*mix;
    uint8_t r = (uint8_t)(mr * f);
    uint8_t g = (uint8_t)(mg * f);
    uint8_t b = (uint8_t)(mb * f);
    if(r<3) r=0; if(g<3) g=0; if(b<3) b=0;
    set_rgb(r,g,b);
  }else{
    uint8_t b = (uint8_t)(f * 255.0f);
    if(b<3) b=0;
    set_rgb(0,0,b);
  }
}
