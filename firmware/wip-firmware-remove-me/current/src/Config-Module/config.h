#pragma once
#include <stdint.h>
#include <stdbool.h>

namespace cfg {
constexpr uint8_t kDeviceNameLen = 32;
constexpr uint8_t kManufacturerLen = 32;
constexpr uint8_t kSerialLen = 16;
constexpr uint8_t kProfileCountMax = 5;
constexpr uint8_t kProfileNameLen = 16;
constexpr uint8_t kDpiSteps = 5;
constexpr uint8_t kButtons = 8;
constexpr uint32_t kConfigVersion = 2;

struct PB7Config {
  uint8_t mode;
  uint8_t trigger;
  uint16_t threshold;
  uint8_t action;
  uint8_t debounce;
  bool invert;
  uint16_t hold_ms;
  uint16_t release_ms;
  uint16_t double_tap_ms;
  uint16_t long_press_ms;
  uint8_t reserved[2];
};

struct Config {
  char device_name[kDeviceNameLen];
  char manufacturer[kManufacturerLen];
  char serial[kSerialLen];
  uint32_t fw_version;
  uint16_t vid;
  uint16_t pid;
  bool en_mcu;
  bool en_usb;
  bool en_config;
  bool en_tactile;
  bool en_lights;
  bool en_hyprx;
  bool en_sensor;
  uint16_t dpi;
  uint16_t dpi_steps[kDpiSteps];
  bool dpi_separate;
  uint16_t dpi_x;
  uint16_t dpi_y;
  uint8_t lod_mm;
  uint8_t lod_squal;
  uint16_t polling_rate;
  bool angle_snap;
  uint8_t angle_strength;
  bool ripple;
  bool motion_sync;
  int16_t sensor_rot;
  uint8_t surface;
  uint8_t smoothing;
  uint8_t jitter;
  bool idle_killer;
  bool idle_zero;
  bool lift_en;
  uint8_t lift_low;
  uint8_t lift_high;
  uint8_t axis_lock;
  bool diag_stab;
  uint8_t debounce_pb3;
  uint8_t debounce_pb4;
  uint8_t debounce_pb5;
  uint8_t debounce_pb6;
  uint8_t debounce_pb7;
  uint8_t click_mode;
  bool slam_click;
  bool drag_click;
  uint8_t button_remap[kButtons];
  uint8_t wheel_div;
  bool wheel_invert;
  bool wheel_smooth;
  uint8_t wheel_debounce;
  bool hyprx_enable;
  bool hyprx_change_led;
  uint8_t hyprx_div_shift;
  uint8_t hyprx_pending_max;
  uint16_t hyprx_hold_ticks;
  uint8_t hyprx_default_target;
  bool hyprx_scroll_to_click;
  uint8_t hyprx_activation;
  uint8_t hyprx_curve;
  uint32_t led_default;
  uint32_t led_macro;
  uint8_t led_brightness;
  uint8_t led_effect;
  uint8_t led_breathing_speed;
  bool led_reactive;
  bool led_off_on_motion;
  uint16_t led_off_timeout;
  PB7Config pb7;
  uint8_t active_profile;
  uint8_t profile_count;
  char profile_name[kProfileCountMax][kProfileNameLen];
  uint16_t profile_dpi[kProfileCountMax];
  uint32_t profile_led[kProfileCountMax];
  uint16_t profile_polling[kProfileCountMax];
  bool profile_angle[kProfileCountMax];
  uint8_t profile_lod[kProfileCountMax];
  uint8_t profile_surface[kProfileCountMax];
  uint32_t config_version;
  bool sof_sync;
  bool partial;
  uint32_t clock_mhz;
  uint32_t scheduler_hz;
  uint32_t crc32;
} __attribute__((packed));

enum class FieldId : uint16_t {
  DeviceName = 1,
  Manufacturer,
  Serial,
  FwVersion,
  Vid,
  Pid,
  EnMcu,
  EnUsb,
  EnConfig,
  EnTactile,
  EnLights,
  EnHyprx,
  Dpi,
  DpiSteps,
  DpiSeparate,
  DpiX,
  DpiY,
  LodMm,
  LodSqual,
  PollingRate,
  AngleSnap,
  AngleStrength,
  Ripple,
  MotionSync,
  SensorRot,
  Surface,
  Smoothing,
  Jitter,
  IdleKiller,
  IdleZero,
  LiftEn,
  LiftLow,
  LiftHigh,
  AxisLock,
  DiagStab,
  DebouncePb3,
  DebouncePb4,
  DebouncePb5,
  DebouncePb6,
  DebouncePb7,
  ClickMode,
  Slam,
  Drag,
  ButtonRemap,
  WheelDiv,
  WheelInvert,
  WheelSmooth,
  WheelDebounce,
  HyprxEnable,
  HyprxChangeLed,
  HyprxDivShift,
  HyprxPendingMax,
  HyprxHoldTicks,
  HyprxDefaultTarget,
  HyprxScrollToClick,
  HyprxActivation,
  HyprxCurve,
  LedDefault,
  LedMacro,
  LedBrightness,
  LedEffect,
  LedBreathingSpeed,
  LedReactive,
  LedOffOnMotion,
  LedOffTimeout,
  Pb7Mode,
  Pb7Trigger,
  Pb7Threshold,
  Pb7Action,
  Pb7Debounce,
  Pb7Invert,
  Pb7HoldMs,
  Pb7ReleaseMs,
  ActiveProfile,
  ProfileCount,
  ProfileDpi,
  ProfileLed,
  ProfilePolling,
  ProfileAngle,
  ProfileLod,
  ProfileSurface,
  ConfigVersion,
  SofSync,
  Partial,
  ClockMhz,
  SchedulerHz
};

enum class SetResult : uint8_t {
  Ok = 0,
  InvalidId,
  OutOfRange,
  ReadOnly,
  ValidationFailed
};

const Config* cfg_get();
Config* cfg_get_mut();
void cfg_init();
bool cfg_load();
bool cfg_save();
SetResult cfg_set_field(FieldId id, int32_t value, uint8_t index);
SetResult cfg_set_string(FieldId id, const char* str, uint8_t index);
bool cfg_validate(const Config* c);
uint32_t cfg_compute_crc(const Config* c);
bool cfg_apply_profile(uint8_t idx);
}
