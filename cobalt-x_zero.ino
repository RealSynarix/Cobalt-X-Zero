#include "system.h"
#include "timer.h"
#include "pipeline.h"
#include "usb.h"
#include "usb_hid.h"

void setup() {
  SystemClock_Config();
  pipeline_init();
  timer_init();
  usb_init();
  usb_hid_init();
}

void loop() {
  usb_sync_tick();

  uint8_t next_btn_state;
  if (pipeline_dequeue(&next_btn_state)) {
    usb_hid_send_report(next_btn_state);
  }
}