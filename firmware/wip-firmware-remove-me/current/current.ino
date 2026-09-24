#pragma once
#include <Arduino.h>
#include "src/MCU-Module/clock.h"
#include "src/MCU-Module/scheduler.h"
#include "src/USB-Module/device.h"
#include "src/USB-Module/hid.h"
#include "src/Config-Module/config.h"
#include "src/Config-Module/webhid.h"
#include "src/Input-Module/tactile.h"
#include "src/Lights-Module/indicator.h"
#include "src/HyprX-Module/engine.h"

using namespace mcu;
using namespace usb;
using namespace cfg;
using namespace input;
using namespace lights;
using namespace hyprx;

Clock g_clock;
Scheduler g_sched;
Device g_usb;
Hid g_hid;
Tactile g_tactile;
Indicator g_led;
Engine g_hyprx;

void setup() {
  g_clock.init();
  cfg_init();
  cfg_load();
  g_sched.init(cfg_get()->scheduler_hz);
  g_usb.init();
  g_hid.init();
  g_tactile.init();
  g_led.init();
  g_hyprx.init();
  webhid_init();
}

void loop() {
  g_sched.tick();
  g_tactile.poll();
  g_hyprx.update();
  g_led.update();
  g_usb.process();
  webhid_process();
}
