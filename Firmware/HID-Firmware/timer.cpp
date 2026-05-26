#include "timer.h"
#include "pipeline.h"
#include <HardwareTimer.h>

static HardwareTimer *htim = nullptr;

static void timer_callback(void) {
  pipeline_tick();
}

void timer_init(void) {
  htim = new HardwareTimer(TIM3);
  htim->setOverflow(32000, HERTZ_FORMAT);
  htim->attachInterrupt(timer_callback);
  htim->setInterruptPriority(0, 0);
  htim->resume();
}

void timer_resync(void) {
  if (htim) {
    TIM3->CNT = 0;
  }
}