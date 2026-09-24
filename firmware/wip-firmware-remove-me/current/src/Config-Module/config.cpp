#include "config.h"
#include <string.h>
#include <Arduino.h>

namespace cfg {
static Config g_cfg;
static uint32_t crc32_table[256];
static bool table_ready = false;

static void crc32_init_table() {
  if (table_ready) return;
  for (uint32_t i = 0; i < 256; ++i) {
    uint32_t c = i;
    for (uint8_t k = 0; k < 8; ++k) c = (c & 1) ? (0xEDB88320UL ^ (c >> 1)) : (c >> 1);
    crc32_table[i] = c;
  }
  table_ready = true;
}

uint32_t cfg_compute_crc(const Config* c) {
  crc32_init_table();
  const uint8_t* p = reinterpret_cast<const uint8_t*>(c);
  size_t len = sizeof(Config) - sizeof(uint32_t);
  uint32_t crc = 0xFFFFFFFFUL;
  for (size_t i = 0; i < len; ++i) crc = crc32_table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
  return crc ^ 0xFFFFFFFFUL;
}

const Config* cfg_get() { return &g_cfg; }
Config* cfg_get_mut() { return &g_cfg; }

static void set_str(char* dst, size_t max, const char* src) {
  memset(dst, 0, max);
  if (!src) return;
  strncpy(dst, src, max - 1);
}

void cfg_init() {
  memset(&g_cfg, 0, sizeof(g_cfg));
  set_str(g_cfg.device_name, kDeviceNameLen, "HyprX Mouse");
  set_str(g_cfg.manufacturer, kManufacturerLen, "HyprX Labs");
  set_str(g_cfg.serial, kSerialLen, "HX00000001");
  g_cfg.fw_version = 0x00020001;
  g_cfg.vid = 0x1209;
  g_cfg.pid = 0xC0BA;
  g_cfg.en_mcu = true;
  g_cfg.en_usb = true;
  g_cfg.en_config = true;
  g_cfg.en_tactile = true;
  g_cfg.en_lights = true;
  g_cfg.en_hyprx = true;
  g_cfg.en_sensor = false;
  g_cfg.dpi = 1600;
  g_cfg.dpi_steps[0] = 400; g_cfg.dpi_steps[1] = 800; g_cfg.dpi_steps[2] = 1600; g_cfg.dpi_steps[3] = 3200; g_cfg.dpi_steps[4] = 6400;
  g_cfg.dpi_separate = false;
  g_cfg.dpi_x = 1600;
  g_cfg.dpi_y = 1600;
  g_cfg.lod_mm = 2;
  g_cfg.lod_squal = 32;
  g_cfg.polling_rate = 1000;
  g_cfg.angle_snap = false;
  g_cfg.angle_strength = 0;
  g_cfg.ripple = false;
  g_cfg.motion_sync = true;
  g_cfg.sensor_rot = 0;
  g_cfg.surface = 0;
  g_cfg.smoothing = 0;
  g_cfg.jitter = 0;
  g_cfg.idle_killer = true;
  g_cfg.idle_zero = true;
  g_cfg.lift_en = true;
  g_cfg.lift_low = 2;
  g_cfg.lift_high = 4;
  g_cfg.axis_lock = 0;
  g_cfg.diag_stab = false;
  g_cfg.debounce_pb3 = 4; g_cfg.debounce_pb4 = 4; g_cfg.debounce_pb5 = 4; g_cfg.debounce_pb6 = 4; g_cfg.debounce_pb7 = 6;
  g_cfg.click_mode = 0;
  g_cfg.slam_click = false;
  g_cfg.drag_click = false;
  for (uint8_t i = 0; i < kButtons; ++i) g_cfg.button_remap[i] = i;
  g_cfg.wheel_div = 1;
  g_cfg.wheel_invert = false;
  g_cfg.wheel_smooth = true;
  g_cfg.wheel_debounce = 2;
  g_cfg.hyprx_enable = true;
  g_cfg.hyprx_change_led = true;
  g_cfg.hyprx_div_shift = 2;
  g_cfg.hyprx_pending_max = 4;
  g_cfg.hyprx_hold_ticks = 120;
  g_cfg.hyprx_default_target = 0;
  g_cfg.hyprx_scroll_to_click = true;
  g_cfg.hyprx_activation = 1;
  g_cfg.hyprx_curve = 1;
  g_cfg.led_default = 0x00FF88;
  g_cfg.led_macro = 0xFF0044;
  g_cfg.led_brightness = 180;
  g_cfg.led_effect = 0;
  g_cfg.led_breathing_speed = 20;
  g_cfg.led_reactive = true;
  g_cfg.led_off_on_motion = false;
  g_cfg.led_off_timeout = 0;
  g_cfg.pb7.mode = 1;
  g_cfg.pb7.trigger = 1;
  g_cfg.pb7.threshold = 1200;
  g_cfg.pb7.action = 1;
  g_cfg.pb7.debounce = 6;
  g_cfg.pb7.invert = false;
  g_cfg.pb7.hold_ms = 300;
  g_cfg.pb7.release_ms = 80;
  g_cfg.pb7.double_tap_ms = 250;
  g_cfg.pb7.long_press_ms = 600;
  g_cfg.active_profile = 0;
  g_cfg.profile_count = 5;
  for (uint8_t i = 0; i < kProfileCountMax; ++i) {
    char tmp[16]; snprintf(tmp, sizeof(tmp), "Profile %u", (unsigned)i + 1);
    set_str(g_cfg.profile_name[i], kProfileNameLen, tmp);
    g_cfg.profile_dpi[i] = g_cfg.dpi_steps[i % kDpiSteps];
    g_cfg.profile_led[i] = g_cfg.led_default;
    g_cfg.profile_polling[i] = g_cfg.polling_rate;
    g_cfg.profile_angle[i] = false;
    g_cfg.profile_lod[i] = g_cfg.lod_mm;
    g_cfg.profile_surface[i] = g_cfg.surface;
  }
  g_cfg.config_version = kConfigVersion;
  g_cfg.sof_sync = true;
  g_cfg.partial = false;
  g_cfg.clock_mhz = 120;
  g_cfg.scheduler_hz = 4000;
  g_cfg.crc32 = cfg_compute_crc(&g_cfg);
}

bool cfg_validate(const Config* c) {
  if (!c) return false;
  if (c->vid == 0 || c->pid == 0) return false;
  if (c->dpi < 100 || c->dpi > 26000) return false;
  for (uint8_t i = 0; i < kDpiSteps; ++i) if (c->dpi_steps[i] < 100 || c->dpi_steps[i] > 26000) return false;
  if (c->dpi_x < 100 || c->dpi_x > 26000) return false;
  if (c->dpi_y < 100 || c->dpi_y > 26000) return false;
  if (c->lod_mm < 1 || c->lod_mm > 5) return false;
  if (c->polling_rate != 125 && c->polling_rate != 250 && c->polling_rate != 500 && c->polling_rate != 1000 && c->polling_rate != 2000 && c->polling_rate != 4000 && c->polling_rate != 8000) return false;
  if (c->angle_strength > 100) return false;
  if (c->sensor_rot < -180 || c->sensor_rot > 180) return false;
  if (c->surface > 5) return false;
  if (c->smoothing > 10) return false;
  if (c->jitter > 20) return false;
  if (c->lift_low > 100 || c->lift_high > 100) return false;
  if (c->lift_low >= c->lift_high) return false;
  if (c->axis_lock > 3) return false;
  if (c->debounce_pb3 > 50 || c->debounce_pb4 > 50 || c->debounce_pb5 > 50 || c->debounce_pb6 > 50 || c->debounce_pb7 > 50) return false;
  if (c->click_mode > 3) return false;
  for (uint8_t i = 0; i < kButtons; ++i) if (c->button_remap[i] >= kButtons) return false;
  if (c->wheel_div == 0 || c->wheel_div > 8) return false;
  if (c->wheel_debounce > 50) return false;
  if (c->hyprx_div_shift > 7) return false;
  if (c->hyprx_pending_max == 0 || c->hyprx_pending_max > 16) return false;
  if (c->hyprx_hold_ticks > 5000) return false;
  if (c->hyprx_default_target >= kButtons) return false;
  if (c->hyprx_activation > 2) return false;
  if (c->hyprx_curve > 4) return false;
  if (c->led_brightness > 255) return false;
  if (c->led_effect > 10) return false;
  if (c->led_breathing_speed == 0 || c->led_breathing_speed > 100) return false;
  if (c->led_off_timeout > 60000) return false;
  if (c->pb7.mode > 3 || c->pb7.trigger > 3 || c->pb7.threshold > 4095 || c->pb7.action > 15 || c->pb7.debounce > 100) return false;
  if (c->active_profile >= kProfileCountMax) return false;
  if (c->profile_count == 0 || c->profile_count > kProfileCountMax) return false;
  if (c->clock_mhz < 48 || c->clock_mhz > 240) return false;
  if (c->scheduler_hz < 1000 || c->scheduler_hz > 8000) return false;
  if (c->en_sensor) return false;
  return true;
}

bool cfg_load() { return true; }
bool cfg_save() {
  if (!cfg_validate(&g_cfg)) return false;
  g_cfg.crc32 = cfg_compute_crc(&g_cfg);
  return true;
}

SetResult cfg_set_field(FieldId id, int32_t value, uint8_t index) {
  Config tmp = g_cfg;
  switch (id) {
    case FieldId::FwVersion: tmp.fw_version = (uint32_t)value; break;
    case FieldId::Vid: if (value <= 0 || value > 0xFFFF) return SetResult::OutOfRange; tmp.vid = (uint16_t)value; break;
    case FieldId::Pid: if (value <= 0 || value > 0xFFFF) return SetResult::OutOfRange; tmp.pid = (uint16_t)value; break;
    case FieldId::EnMcu: tmp.en_mcu = value != 0; break;
    case FieldId::EnUsb: tmp.en_usb = value != 0; break;
    case FieldId::EnConfig: tmp.en_config = value != 0; break;
    case FieldId::EnTactile: tmp.en_tactile = value != 0; break;
    case FieldId::EnLights: tmp.en_lights = value != 0; break;
    case FieldId::EnHyprx: tmp.en_hyprx = value != 0; break;
    case FieldId::Dpi: tmp.dpi = (uint16_t)value; break;
    case FieldId::DpiSteps: if (index >= kDpiSteps) return SetResult::OutOfRange; tmp.dpi_steps[index] = (uint16_t)value; break;
    case FieldId::DpiSeparate: tmp.dpi_separate = value != 0; break;
    case FieldId::DpiX: tmp.dpi_x = (uint16_t)value; break;
    case FieldId::DpiY: tmp.dpi_y = (uint16_t)value; break;
    case FieldId::LodMm: tmp.lod_mm = (uint8_t)value; break;
    case FieldId::LodSqual: tmp.lod_squal = (uint8_t)value; break;
    case FieldId::PollingRate: tmp.polling_rate = (uint16_t)value; break;
    case FieldId::AngleSnap: tmp.angle_snap = value != 0; break;
    case FieldId::AngleStrength: tmp.angle_strength = (uint8_t)value; break;
    case FieldId::Ripple: tmp.ripple = value != 0; break;
    case FieldId::MotionSync: tmp.motion_sync = value != 0; break;
    case FieldId::SensorRot: tmp.sensor_rot = (int16_t)value; break;
    case FieldId::Surface: tmp.surface = (uint8_t)value; break;
    case FieldId::Smoothing: tmp.smoothing = (uint8_t)value; break;
    case FieldId::Jitter: tmp.jitter = (uint8_t)value; break;
    case FieldId::IdleKiller: tmp.idle_killer = value != 0; break;
    case FieldId::IdleZero: tmp.idle_zero = value != 0; break;
    case FieldId::LiftEn: tmp.lift_en = value != 0; break;
    case FieldId::LiftLow: tmp.lift_low = (uint8_t)value; break;
    case FieldId::LiftHigh: tmp.lift_high = (uint8_t)value; break;
    case FieldId::AxisLock: tmp.axis_lock = (uint8_t)value; break;
    case FieldId::DiagStab: tmp.diag_stab = value != 0; break;
    case FieldId::DebouncePb3: tmp.debounce_pb3 = (uint8_t)value; break;
    case FieldId::DebouncePb4: tmp.debounce_pb4 = (uint8_t)value; break;
    case FieldId::DebouncePb5: tmp.debounce_pb5 = (uint8_t)value; break;
    case FieldId::DebouncePb6: tmp.debounce_pb6 = (uint8_t)value; break;
    case FieldId::DebouncePb7: tmp.debounce_pb7 = (uint8_t)value; break;
    case FieldId::ClickMode: tmp.click_mode = (uint8_t)value; break;
    case FieldId::Slam: tmp.slam_click = value != 0; break;
    case FieldId::Drag: tmp.drag_click = value != 0; break;
    case FieldId::ButtonRemap: if (index >= kButtons) return SetResult::OutOfRange; tmp.button_remap[index] = (uint8_t)value; break;
    case FieldId::WheelDiv: tmp.wheel_div = (uint8_t)value; break;
    case FieldId::WheelInvert: tmp.wheel_invert = value != 0; break;
    case FieldId::WheelSmooth: tmp.wheel_smooth = value != 0; break;
    case FieldId::WheelDebounce: tmp.wheel_debounce = (uint8_t)value; break;
    case FieldId::HyprxEnable: tmp.hyprx_enable = value != 0; break;
    case FieldId::HyprxChangeLed: tmp.hyprx_change_led = value != 0; break;
    case FieldId::HyprxDivShift: tmp.hyprx_div_shift = (uint8_t)value; break;
    case FieldId::HyprxPendingMax: tmp.hyprx_pending_max = (uint8_t)value; break;
    case FieldId::HyprxHoldTicks: tmp.hyprx_hold_ticks = (uint16_t)value; break;
    case FieldId::HyprxDefaultTarget: tmp.hyprx_default_target = (uint8_t)value; break;
    case FieldId::HyprxScrollToClick: tmp.hyprx_scroll_to_click = value != 0; break;
    case FieldId::HyprxActivation: tmp.hyprx_activation = (uint8_t)value; break;
    case FieldId::HyprxCurve: tmp.hyprx_curve = (uint8_t)value; break;
    case FieldId::LedDefault: tmp.led_default = (uint32_t)value; break;
    case FieldId::LedMacro: tmp.led_macro = (uint32_t)value; break;
    case FieldId::LedBrightness: tmp.led_brightness = (uint8_t)value; break;
    case FieldId::LedEffect: tmp.led_effect = (uint8_t)value; break;
    case FieldId::LedBreathingSpeed: tmp.led_breathing_speed = (uint8_t)value; break;
    case FieldId::LedReactive: tmp.led_reactive = value != 0; break;
    case FieldId::LedOffOnMotion: tmp.led_off_on_motion = value != 0; break;
    case FieldId::LedOffTimeout: tmp.led_off_timeout = (uint16_t)value; break;
    case FieldId::Pb7Mode: tmp.pb7.mode = (uint8_t)value; break;
    case FieldId::Pb7Trigger: tmp.pb7.trigger = (uint8_t)value; break;
    case FieldId::Pb7Threshold: tmp.pb7.threshold = (uint16_t)value; break;
    case FieldId::Pb7Action: tmp.pb7.action = (uint8_t)value; break;
    case FieldId::Pb7Debounce: tmp.pb7.debounce = (uint8_t)value; break;
    case FieldId::Pb7Invert: tmp.pb7.invert = value != 0; break;
    case FieldId::Pb7HoldMs: tmp.pb7.hold_ms = (uint16_t)value; break;
    case FieldId::Pb7ReleaseMs: tmp.pb7.release_ms = (uint16_t)value; break;
    case FieldId::ActiveProfile: tmp.active_profile = (uint8_t)value; break;
    case FieldId::ProfileCount: tmp.profile_count = (uint8_t)value; break;
    case FieldId::ProfileDpi: if (index >= kProfileCountMax) return SetResult::OutOfRange; tmp.profile_dpi[index] = (uint16_t)value; break;
    case FieldId::ProfileLed: if (index >= kProfileCountMax) return SetResult::OutOfRange; tmp.profile_led[index] = (uint32_t)value; break;
    case FieldId::ProfilePolling: if (index >= kProfileCountMax) return SetResult::OutOfRange; tmp.profile_polling[index] = (uint16_t)value; break;
    case FieldId::ProfileAngle: if (index >= kProfileCountMax) return SetResult::OutOfRange; tmp.profile_angle[index] = value != 0; break;
    case FieldId::ProfileLod: if (index >= kProfileCountMax) return SetResult::OutOfRange; tmp.profile_lod[index] = (uint8_t)value; break;
    case FieldId::ProfileSurface: if (index >= kProfileCountMax) return SetResult::OutOfRange; tmp.profile_surface[index] = (uint8_t)value; break;
    case FieldId::ConfigVersion: return SetResult::ReadOnly;
    case FieldId::SofSync: tmp.sof_sync = value != 0; break;
    case FieldId::Partial: tmp.partial = value != 0; break;
    case FieldId::ClockMhz: tmp.clock_mhz = (uint32_t)value; break;
    case FieldId::SchedulerHz: tmp.scheduler_hz = (uint32_t)value; break;
    default: return SetResult::InvalidId;
  }
  if (!cfg_validate(&tmp)) return SetResult::ValidationFailed;
  g_cfg = tmp;
  g_cfg.crc32 = cfg_compute_crc(&g_cfg);
  return SetResult::Ok;
}

SetResult cfg_set_string(FieldId id, const char* str, uint8_t index) {
  if (!str) return SetResult::OutOfRange;
  Config tmp = g_cfg;
  size_t len = strlen(str);
  switch (id) {
    case FieldId::DeviceName: if (len >= kDeviceNameLen) return SetResult::OutOfRange; set_str(tmp.device_name, kDeviceNameLen, str); break;
    case FieldId::Manufacturer: if (len >= kManufacturerLen) return SetResult::OutOfRange; set_str(tmp.manufacturer, kManufacturerLen, str); break;
    case FieldId::Serial: if (len >= kSerialLen) return SetResult::OutOfRange; set_str(tmp.serial, kSerialLen, str); break;
    default: return SetResult::InvalidId;
  }
  if (!cfg_validate(&tmp)) return SetResult::ValidationFailed;
  g_cfg = tmp;
  g_cfg.crc32 = cfg_compute_crc(&g_cfg);
  return SetResult::Ok;
}

bool cfg_apply_profile(uint8_t idx) {
  if (idx >= g_cfg.profile_count) return false;
  g_cfg.dpi = g_cfg.profile_dpi[idx];
  g_cfg.polling_rate = g_cfg.profile_polling[idx];
  g_cfg.angle_snap = g_cfg.profile_angle[idx];
  g_cfg.lod_mm = g_cfg.profile_lod[idx];
  g_cfg.surface = g_cfg.profile_surface[idx];
  g_cfg.led_default = g_cfg.profile_led[idx];
  g_cfg.active_profile = idx;
  g_cfg.crc32 = cfg_compute_crc(&g_cfg);
  return true;
}
}
