#include "scheduler.h"
#include <HardwareTimer.h>
static HardwareTimer *tim=nullptr;
static void cb(void){}
void scheduler_init(void){
tim=new HardwareTimer(TIM6);
tim->setOverflow(1000,HERTZ_FORMAT);
tim->attachInterrupt(cb);
tim->setInterruptPriority(2,0);
tim->resume();
}
