#include "src/MCU-Module/clock.h"
#include "src/MCU-Module/scheduler.h"
#include "src/USB-Module/device.h"
#include "src/USB-Module/hid.h"
#include "src/Input-Module/tactile.h"
#include "src/Lights-Module/indicator.h"
void setup(void){
  clock_init();
  tactile_init();
  indicator_init();
  hid_init();
  usb_device_init();
  scheduler_init();
}
void loop(void){
  static uint32_t last_ind=0;
  for(;;){

    if(usb_device_sof()){
      tactile_report_t tr;
      tactile_pop(&tr);
      hid_send(tr.buttons, tr.wheel, 0, 0);
    }

    uint32_t now = millis();
    if(now - last_ind >= 50){
      last_ind = now;
      indicator_tick();
    }
  }
}
