#include "src/MCU-Module/clock.h"
#include "src/MCU-Module/scheduler.h"
#include "src/USB-Module/device.h"
#include "src/USB-Module/hid.h"
#include "src/Input-Module/tactile.h"
#include "src/Lights-Module/indicator.h"
#include "src/HyprX-Module/engine.h"
#include "src/Config-Module/config.h"
#include "src/Sensor-Module/sensor.h"

static uint32_t last_ind=0;
static uint8_t last_buttons=0;

void setup(void){
  clock_init();
  config_init();
  tactile_init();
  indicator_init();
  hid_init();
  sensor_init();
  usb_device_init();
  scheduler_init();
}

void loop(void){
  for(;;){
    usb_device_poll();
    sensor_task();
    hid_task();
    if(hid_slot()){
      tactile_report_t tr={0,0,0};
      uint8_t has=tactile_pop(&tr);
      int16_t dx,dy; sensor_take(&dx,&dy);
      if(has){
        if(tr.macro) hid_type_program_me();
        last_buttons=tr.buttons;
        hid_send(last_buttons, tr.wheel, dx, dy);
      }else{
        hid_send(last_buttons, 0, dx, dy);
      }
    }
    hid_service();
    uint32_t now=millis();
    if(now-last_ind>=20){ last_ind=now; indicator_tick(); }
  }
}
