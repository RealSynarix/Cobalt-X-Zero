#include "scheduler.h"
#include "../Input-Module/tactile.h"
#include <HardwareTimer.h>
static HardwareTimer *tim=nullptr;
static void cb(void){ tactile_tick(); }
void scheduler_init(void){
  tim=new HardwareTimer(TIM6);
  tim->setOverflow(64000,HERTZ_FORMAT);
  tim->attachInterrupt(cb);
  tim->setInterruptPriority(3,0);
  tim->resume();
}
