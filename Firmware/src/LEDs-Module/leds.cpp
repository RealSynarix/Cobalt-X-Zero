#include "leds.h"
#include "../HyprX-Module/macro.h"
#include <Arduino.h>
#include <math.h>

#define PI 3.1415926f
#define CYCLE_MS 10000u

static void set_rgb(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(PA8, r);
  analogWrite(PA9, g);
  analogWrite(PA10, b);
}

void leds_init(void) {
  pinMode(PA8, OUTPUT);
  pinMode(PA9, OUTPUT);
  pinMode(PA10, OUTPUT);
  set_rgb(0, 0, 0);
}

void leds_tick(void) {
  uint32_t t = millis() % CYCLE_MS;
  float Ra, Ga, Ba, Rb, Gb, Bb;
  if (macro_is_hyprx_active()) {
    Ra = 192.0f;
    Ga = 0.0f;
    Ba = 10.0f;
    Rb = 255.0f;
    Gb = 90.0f;
    Bb = 0.0f;
  } else {
    Ra = 10.0f;
    Ga = 60.0f;
    Ba = 200.0f;
    Rb = 8.0f;
    Gb = 80.0f;
    Bb = 190.0f;
  }
  float r, g, b;
  if (t < 3000u) {
    r = Ra;
    g = Ga;
    b = Ba;
  } else if (t < 4000u) {
    float p = (float)(t - 3000u) / 1000.0f;
    float e = 0.5f + 0.5f * cosf(p * PI);
    r = Ra * e;
    g = Ga * e;
    b = Ba * e;
  } else if (t < 5000u) {
    float p = (float)(t - 4000u) / 1000.0f;
    float e = 0.5f - 0.5f * cosf(p * PI);
    r = Rb * e;
    g = Gb * e;
    b = Bb * e;
  } else if (t < 8000u) {
    r = Rb;
    g = Gb;
    b = Bb;
  } else if (t < 9000u) {
    float p = (float)(t - 8000u) / 1000.0f;
    float e = 0.5f + 0.5f * cosf(p * PI);
    r = Rb * e;
    g = Gb * e;
    b = Bb * e;
  } else {
    float p = (float)(t - 9000u) / 1000.0f;
    float e = 0.5f - 0.5f * cosf(p * PI);
    r = Ra * e;
    g = Ga * e;
    b = Ba * e;
  }
  set_rgb((uint8_t)r, (uint8_t)g, (uint8_t)b);
}
