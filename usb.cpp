#include "usb.h"
#include "timer.h"
#include <stm32g4xx_hal.h>

void usb_init(void) {
  USB->CNTR |= USB_CNTR_SOFM;
  USB->ISTR &= ~USB_ISTR_SOF;
}

void usb_sync_tick(void) {
  if (USB->ISTR & USB_ISTR_SOF) {
    USB->ISTR &= ~USB_ISTR_SOF;
    timer_resync();
  }
}