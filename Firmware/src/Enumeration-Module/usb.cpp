#include "usb.h"
#include "../Core-Module/timer.h"
#include <stm32g4xx_hal.h>

static uint16_t last_frame = 0xFFFF;

void usb_init(void) {
  last_frame = 0xFFFF;
}

uint8_t usb_sof_detected(void) {
  uint16_t current = (uint16_t)(USB->FNR & USB_FNR_FN);
  if (current!= last_frame) {
    last_frame = current;
    timer_resync();
    return 1;
  }
  return 0;
}
