#include "system.h"
#include "usb.h"
#include "timer.h"
#include "pipeline.h"
#include "usb_hid.h"

void setup(void) {
  pipeline_init();
  usb_hid_init();
  usb_init();
  timer_init();
}

void loop(void) {
  usb_sync_tick();

  pipeline_report_t report;
  if (pipeline_dequeue(&report)) {
    usb_hid_send_report(report.buttons, report.wheel);
  }
}