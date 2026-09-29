#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif
#define FIELD_DEVICE_NAME 1
#define FIELD_MANUFACTURER 2
#define FIELD_SERIAL 3
#define FIELD_FW_VERSION 4
#define FIELD_POLL 22
#define FIELD_SOF_SYNC 23
#define FIELD_DPI 32
#define FIELD_SENSOR_POLL 33
#define FIELD_LOD_MM 34
#define FIELD_LOD_SQUAL 35
#define FIELD_ANGLE_SNAP 36
#define FIELD_ANGLE_STRENGTH 37
#define FIELD_SURFACE 38
#define FIELD_GLITCH 39
#define FIELD_MOTION_SMOOTH 40
#define FIELD_BURST_WINDOW 41
#define FIELD_LOW_RESPONSE 42
#define FIELD_HIGH_RESPONSE 43
#define FIELD_TRACK_HYST 44
#define FIELD_DB_PB3 48
#define FIELD_DB_PB4 49
#define FIELD_DB_PB5 50
#define FIELD_DB_PB6 51
#define FIELD_DB_PB7 52
#define FIELD_CLICK_MODE 53
#define FIELD_DEBOUNCE_SCALE 54
#define FIELD_BUTTON_HOLD 55
#define FIELD_WHEEL_DIV 64
#define FIELD_WHEEL_INV 65
#define FIELD_WHEEL_SMOOTH 66
#define FIELD_WHEEL_SMOOTH_STRENGTH 67
#define FIELD_WHEEL_ACCEL 68
#define FIELD_WHEEL_NOISE 69
#define FIELD_HYPRX_EN 80
#define FIELD_HYPRX_LED 81
#define FIELD_HYPRX_TARGET 82
#define FIELD_HYPRX_STC 83
#define FIELD_HYPRX_DIV 84
#define FIELD_HYPRX_HOLD 85
#define FIELD_HYPRX_INTERVAL 86
#define FIELD_HYPRX_MODE 87
#define FIELD_LED_DEF 96
#define FIELD_LED_MACRO 97
#define FIELD_LED_BRI 98
#define FIELD_LED_EFF 99
#define FIELD_LED_SPEED 100
#define FIELD_LED_IDLE 101
#define FIELD_LED_ACTIVITY 102
#define FIELD_PIO_MODE 112
#define FIELD_PIO_TRIGGER 113
#define FIELD_PIO_MOUSE 114
#define FIELD_PIO_KEY 115
#define FIELD_PIO_COMBO 116
#define FIELD_PIO_MEDIA 117
#define FIELD_PIO_TEXT 118
#define FIELD_PIO_BLOCK0 119
#define FIELD_PIO_BLOCK7 126
#define PIO_BLOCKS 8
#define PIO_BLOCK_SIZE 56
#define PIO_MAX_SIZE (PIO_BLOCKS*PIO_BLOCK_SIZE)
#define CONFIG_VERSION 0x00020001u
typedef struct{
char device_name[32];char manufacturer[32];char serial[16];char fw_version[16];
uint16_t vid;uint16_t pid;
uint16_t poll;uint8_t sof_sync;
uint16_t dpi;uint16_t sensor_poll;uint8_t lod_mm;uint8_t lod_squal;uint8_t angle_snap;uint8_t angle_strength;uint8_t surface;
uint16_t glitch_limit;uint8_t motion_smooth;uint8_t burst_window;uint8_t low_response;uint8_t high_response;uint8_t track_hyst;
uint8_t db_pb3;uint8_t db_pb4;uint8_t db_pb5;uint8_t db_pb6;uint8_t db_pb7;uint8_t click_mode;uint8_t debounce_scale;uint8_t button_hold;
uint8_t wheel_div;uint8_t wheel_inv;uint8_t wheel_smooth;uint8_t wheel_smooth_strength;uint8_t wheel_accel;uint8_t wheel_noise;
uint8_t hyprx_en;uint8_t hyprx_led;uint8_t hyprx_target;uint8_t hyprx_stc;uint8_t hyprx_div;uint16_t hyprx_hold;uint16_t hyprx_interval;uint8_t hyprx_mode;
uint8_t led_def[3];uint8_t led_macro[3];uint8_t led_bri;uint8_t led_eff;uint8_t led_speed;uint16_t led_idle;uint8_t led_activity;
uint8_t pio_mode;uint8_t pio_trigger;uint8_t pio_mouse;uint8_t pio_key;uint8_t pio_combo[8];uint8_t pio_combo_len;uint8_t pio_media;char pio_text[56];uint8_t pio_program[PIO_MAX_SIZE];uint8_t reserved[4];
uint32_t config_version;uint32_t crc32;
}config_t;
extern config_t g_cfg;
void config_init(void);int config_load(void);int config_save(void);
int config_set_field(uint8_t fid,uint8_t *data,uint16_t len);
int config_get_field(uint8_t fid,uint8_t *out,uint16_t *out_len);
int config_reset_defaults(void);uint32_t config_crc32(uint8_t *d,uint32_t l);
#ifdef __cplusplus
}
#endif
