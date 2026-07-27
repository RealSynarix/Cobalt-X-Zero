#include "src/Core-Module/system.h"
#include "src/Core-Module/timer.h"
#include "src/Enumeration-Module/usb.h"
#include "src/Enumeration-Module/usb_hid.h"
#include "src/TactileIO-Module/pipeline.h"
#include "src/LEDs-Module/leds.h"

void setup(void) {
  SystemClock_Config();
  pipeline_init();
  leds_init();
  usb_hid_init();
  usb_init();
  timer_init();
}

void loop(void) {
  if (usb_sof_detected()) {
    pipeline_report_t r;
    if (pipeline_get_ready_report(&r)) {
      usb_hid_send_report(r.buttons, r.wheel);
    }
  }
  leds_tick();
}
