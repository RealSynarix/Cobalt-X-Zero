#include "usb.h"
#include "timer.h"
#include <stm32g4xx_hal.h>

static uint16_t last_frame = 0xFFFF;

void usb_init(void) {
  USB->CNTR |= USB_CNTR_SOFM;
  USB->ISTR = (uint16_t)~USB_ISTR_SOF;
}

uint8_t usb_sof_detected(void) {
  uint16_t current_frame = (uint16_t)(USB->FNR & USB_FNR_FN);
  if (current_frame != last_frame) {
    last_frame = current_frame;
    timer_resync();
    return 1;
  }
  return 0;
}