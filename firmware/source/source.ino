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
tactile_report_t tr;
tactile_pop(&tr);
int16_t dx,dy;
sensor_take(&dx,&dy);
hid_send(tr.buttons,tr.wheel,dx,dy);
}
hid_service();
uint32_t now=millis();
if(now-last_ind>=20){last_ind=now;indicator_tick();}
}
}
