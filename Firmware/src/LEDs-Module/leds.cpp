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
  float phase = (float)t / CYCLE_MS * 2.0f * PI;
  float f = 0.15f + 0.85f * (0.5f - 0.5f * cosf(phase));

  if(macro_is_hyprx_active()){
    uint8_t r,g,b;
    if(t < 3000u){
      r=200; g=0; b=0;
    }else if(t < 4000u){
      float p=(float)(t-3000u)/1000.0f;
      float e=0.5f-0.5f*cosf(p*PI);
      r=(uint8_t)(200*(1.0f-e)+255*e);
      g=(uint8_t)(0*(1.0f-e)+70*e);
      b=0;
    }else if(t < 8000u){
      r=255; g=70; b=0;
    }else if(t < 9000u){
      float p=(float)(t-8000u)/1000.0f;
      float e=0.5f-0.5f*cosf(p*PI);
      r=(uint8_t)(255*(1.0f-e)+200*e);
      g=(uint8_t)(70*(1.0f-e)+0*e);
      b=0;
    }else{
      r=200; g=0; b=0;
    }
    if(r<4) r=0; if(g<4) g=0; if(b<4) b=0;
    r=(uint8_t)(r*f); g=(uint8_t)(g*f); b=(uint8_t)(b*f);
    if(r>0 && r<15) r=15;
    if(g>0 && g<15) g=0;
    if(b>0 && b<15) b=0;
    if(r==200 || r==255) g= (t>=3000u && t<9000u)? g : 0;
    set_rgb(r,g,b);
  }else{
    uint8_t b = (uint8_t)(f*255.0f);
    if(b<38) b=38;
    set_rgb(0,0,b);
  }
}
