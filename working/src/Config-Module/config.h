#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif
#define FIELD_DEVICE_NAME 1
#define FIELD_MANUFACTURER 2
#define FIELD_SERIAL 3
#define FIELD_FW_VERSION 4
#define FIELD_MOD_SENSOR 16
#define FIELD_MOD_BUTTONS 17
#define FIELD_MOD_SCROLL 18
#define FIELD_MOD_HYPRX 19
#define FIELD_MOD_LEDS 20
#define FIELD_MOD_SOF 21
#define FIELD_DPI 32
#define FIELD_POLL 33
#define FIELD_LOD_MM 34
#define FIELD_LOD_SQUAL 35
#define FIELD_ANGLE_SNAP 36
#define FIELD_ANGLE_STRENGTH 37
#define FIELD_SURFACE 38
#define FIELD_DB_PB3 48
#define FIELD_DB_PB4 49
#define FIELD_DB_PB5 50
#define FIELD_DB_PB6 51
#define FIELD_DB_PB7 52
#define FIELD_CLICK_MODE 53
#define FIELD_WHEEL_DIV 64
#define FIELD_WHEEL_INV 65
#define FIELD_WHEEL_SMOOTH 66
#define FIELD_HYPRX_EN 80
#define FIELD_HYPRX_LED 81
#define FIELD_HYPRX_TARGET 82
#define FIELD_HYPRX_STC 83
#define FIELD_HYPRX_DIV 84
#define FIELD_HYPRX_HOLD 85
#define FIELD_LED_DEF 96
#define FIELD_LED_MACRO 97
#define FIELD_LED_BRI 98
#define FIELD_LED_EFF 99
#define FIELD_LED_SPEED 100
#define FIELD_PB7_MODE 112
#define FIELD_PB7_AT 113
#define FIELD_PB7_MOUSE 114
#define FIELD_PB7_KEY 115
#define FIELD_PB7_COMBO 116
#define FIELD_PB7_MEDIA 117
#define CONFIG_VERSION 0x00010001
typedef struct{
char device_name[32]; char manufacturer[32]; char serial[16]; char fw_version[16];
uint16_t vid; uint16_t pid;
uint8_t mod_sensor; uint8_t mod_buttons; uint8_t mod_scroll; uint8_t mod_hyprx; uint8_t mod_leds; uint8_t mod_sof;
uint16_t dpi; uint16_t poll; uint8_t lod_mm; uint8_t lod_squal; uint8_t angle_snap; uint8_t angle_strength; uint8_t surface;
uint8_t db_pb3; uint8_t db_pb4; uint8_t db_pb5; uint8_t db_pb6; uint8_t db_pb7; uint8_t click_mode;
uint8_t wheel_div; uint8_t wheel_inv; uint8_t wheel_smooth;
uint8_t hyprx_en; uint8_t hyprx_led; uint8_t hyprx_target; uint8_t hyprx_stc; uint8_t hyprx_div; uint16_t hyprx_hold;
uint8_t led_def[3]; uint8_t led_macro[3]; uint8_t led_bri; uint8_t led_eff; uint8_t led_speed;
uint8_t pb7_mode; uint8_t pb7_at; uint8_t pb7_mouse; uint8_t pb7_key; uint8_t pb7_combo[8]; uint8_t pb7_combo_len; uint8_t pb7_media;
uint32_t config_version; uint32_t crc32;
}config_t;
extern config_t g_cfg;
void config_init(void);
int config_load(void);
int config_save(void);
int config_set_field(uint8_t fid,uint8_t *data,uint16_t len);
int config_get_field(uint8_t fid,uint8_t *out,uint16_t *out_len);
int config_reset_defaults(void);
uint32_t config_crc32(uint8_t *d,uint32_t l);
#ifdef __cplusplus
}
#endif
