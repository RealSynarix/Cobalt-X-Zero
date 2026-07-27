#include "timer.h"
#include "../TactileIO-Module/pipeline.h"
#include <HardwareTimer.h>
#include <stm32g4xx.h>

static HardwareTimer timer3(TIM3);
static HardwareTimer *htim = nullptr;

static void timer_callback(void) {
  pipeline_tick();
}

void timer_init(void) {
  htim = &timer3;
  htim->setOverflow(32000, HERTZ_FORMAT);
  htim->attachInterrupt(timer_callback);
  htim->setInterruptPriority(1, 0);
  htim->resume();
}

void timer_resync(void) {
  if (htim!= nullptr) {
    TIM3->CNT = 0;
  }
}
