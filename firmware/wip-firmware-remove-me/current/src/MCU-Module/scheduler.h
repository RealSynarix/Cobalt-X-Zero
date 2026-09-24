#pragma once
#include <stdint.h>

namespace mcu {
using TaskFn = void(*)();
struct Task {
  TaskFn fn;
  uint32_t interval_us;
  uint32_t last_us;
  bool enabled;
};

class Scheduler {
public:
  void init(uint32_t hz);
  void tick();
  bool add_task(TaskFn fn, uint32_t interval_us);
  void set_hz(uint32_t hz);
  uint32_t get_hz() const;
private:
  static constexpr uint8_t kMaxTasks = 16;
  Task tasks_[kMaxTasks];
  uint8_t count_;
  uint32_t hz_;
  uint32_t tick_us_;
};
}
