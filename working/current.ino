#include "src/MCU-Module/clock.h"
#include "src/MCU-Module/scheduler.h"
#include "src/USB-Module/device.h"
#include "src/USB-Module/hid.h"

void setup(void)
{
  clock_init();
  hid_init();
  usb_device_init();
  scheduler_init();
}

void loop(void)
{
  usb_device_sof();
}
