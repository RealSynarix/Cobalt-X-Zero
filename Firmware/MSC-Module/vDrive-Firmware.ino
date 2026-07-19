// BOOTLOADER_FLAG: To flash via custom bootloader instead of Arduino IDE,
// change -DVECT_TAB_OFFSET=0x0 to -DVECT_TAB_OFFSET=0x14000 in build_opt.h

#include <Arduino.h>
#include "system.h"
#include "vdrive.h"

void setup(void) {
  SystemClock_Config();
  vdrive_init();
}

void loop(void) {}