#include <Arduino.h>
#include "vdrive.h"

void setup(void) {
    vdrive_init();
    USBDevice.attach();
}

void loop(void) {
}
