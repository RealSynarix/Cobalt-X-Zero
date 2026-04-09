
#include "vdrive.h"
#include <stm32g4xx.h>

void vdrive_init() {
  if (GPIOA->IDR & (1 << 6)) {
    // MSC
  }
}
