// BOOTLOADER_FLAG: Change 0x0 to 0x4000 when using a bootloader in build_opt.h

#include "system.h"
#include "usb.h"
#include "timer.h"
#include "pipeline.h"
#include "usb_hid.h"

void setup(void) {
  SystemClock_Config();
  delay(10);
  pipeline_init();
  usb_hid_init();
  usb_init();
  timer_init();
}

void loop(void) {
  if (usb_sof_detected()) {
    pipeline_report_t report;
    if (pipeline_get_ready_report(&report)) {
      usb_hid_send_report(report.buttons, report.wheel);
    }
  }
}