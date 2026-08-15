#include "src/MCU-Module/clock.h"
#include "src/MCU-Module/scheduler.h"
#include "src/USB-Module/device.h"
#include "src/USB-Module/hid.h"
#include "src/Input-Module/tactile.h"
#include "src/Lights-Module/indicator.h"

void setup(void) {
  clock_init();
  tactile_init();
  indicator_init();
  hid_init();
  usb_device_init();
  scheduler_init();
}

void loop(void) {
  if (usb_device_sof()) {
    tactile_report_t r;
    tactile_pop(&r);
    hid_send(r.buttons, r.wheel);
  }
  indicator_tick();
}


