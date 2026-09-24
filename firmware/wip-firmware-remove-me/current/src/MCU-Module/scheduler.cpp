#include "scheduler.h"
#include <Arduino.h>

namespace mcu {
void Scheduler::init(uint32_t hz) {
  count_ = 0;
  set_hz(hz);
  for (uint8_t i = 0; i < kMaxTasks; ++i) tasks_[i] = {nullptr, 0, 0, false};
}
void Scheduler::set_hz(uint32_t hz) {
  if (hz < 1000) hz = 1000;
  if (hz > 8000) hz = 8000;
  hz_ = hz;
  tick_us_ = 1000000UL / hz_;
}
uint32_t Scheduler::get_hz() const { return hz_; }
bool Scheduler::add_task(TaskFn fn, uint32_t interval_us) {
  if (count_ >= kMaxTasks || !fn) return false;
  tasks_[count_++] = {fn, interval_us, micros(), true};
  return true;
}
void Scheduler::tick() {
  uint32_t now = micros();
  for (uint8_t i = 0; i < count_; ++i) {
    Task &t = tasks_[i];
    if (!t.enabled || !t.fn) continue;
    if ((now - t.last_us) >= t.interval_us) {
      t.last_us = now;
      t.fn();
    }
  }
}
}
