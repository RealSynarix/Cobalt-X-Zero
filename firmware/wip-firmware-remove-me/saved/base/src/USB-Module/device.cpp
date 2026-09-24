#include "device.h"
#include <stm32g4xx.h>
static volatile uint16_t last_fn = 0xFFFF;
void usb_device_init(void){ last_fn = 0xFFFF; __DSB(); }
uint8_t usb_device_sof(void){
  uint16_t cur = (uint16_t)(USB->FNR & USB_FNR_FN);
  if(cur == last_fn) return 0;
  last_fn = cur;
  return 1;
}
