#include <Arduino.h>
#include <SPI.h>
#include <PMW3360.h>
#include <Mouse.h>
#include "system.h"
#include "usb_hid.h"

#define SENSOR_CS PA4
#define SENSOR_CPI 800

PMW3360 sensor;
static bool sensor_ok = false;

static inline int8_t clamp8(int16_t v) {
  if (v > 127) return 127;
  if (v < -127) return -127;
  return (int8_t)v;
}

void setup() {
  SystemClock_Config();
  usb_hid_init();
  delay(2000);
  pinMode(SENSOR_CS, OUTPUT);
  digitalWrite(SENSOR_CS, HIGH);
  SPI.setMOSI(PA7);
  SPI.setMISO(PA6);
  SPI.setSCLK(PA5);
  for (int i = 0; i < 10 && !sensor_ok; i++) {
    delay(200);
    sensor_ok = sensor.begin(SENSOR_CS);
  }
  if (sensor_ok) {
    sensor.setCPI(SENSOR_CPI);
    Mouse.move(20, 0, 0);
  } else {
    Mouse.move(0, 20, 0);
  }
}

void loop() {
  if (!sensor_ok) return;
  PMW3360_DATA d = sensor.readBurst();
  int8_t mx = clamp8(d.dx);
  int8_t my = clamp8(d.dy);
  if (mx != 0 || my != 0) {
    Mouse.move(mx, my, 0);
  }
}