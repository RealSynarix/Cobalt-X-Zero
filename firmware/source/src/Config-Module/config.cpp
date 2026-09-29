#include "config.h"
#include <string.h>
#include <stm32g4xx_hal.h>
config_t g_cfg;
static uint32_t table[256];
static void crc_init(void){for(uint32_t i=0;i<256;i++){uint32_t c=i;for(int j=0;j<8;j++)c=(c&1)?(0xEDB88320u^(c>>1)):(c>>1);table[i]=c;}}
uint32_t config_crc32(uint8_t *d,uint32_t l){uint32_t c=0xFFFFFFFFu;for(uint32_t i=0;i<l;i++)c=table[(c^d[i])&0xFF]^(c>>8);return c^0xFFFFFFFFu;}
static void defaults(void){
memset(&g_cfg,0,sizeof(g_cfg));
strncpy(g_cfg.device_name,"Cobalt-X Zero",31);
strncpy(g_cfg.manufacturer,"Synarix",31);
strncpy(g_cfg.serial,"CXZ-000001",15);
strncpy(g_cfg.fw_version,"2.0.0",15);
g_cfg.vid=0x1209;g_cfg.pid=0xC0BA;
g_cfg.poll=1000;g_cfg.sof_sync=1;
g_cfg.dpi=800;g_cfg.sensor_poll=8000;g_cfg.lod_mm=2;g_cfg.lod_squal=20;g_cfg.angle_snap=0;g_cfg.angle_strength=0;g_cfg.surface=0;
g_cfg.glitch_limit=900;g_cfg.motion_smooth=85;g_cfg.burst_window=8;g_cfg.low_response=30;g_cfg.high_response=100;g_cfg.track_hyst=2;
g_cfg.db_pb3=0;g_cfg.db_pb4=0;g_cfg.db_pb5=0;g_cfg.db_pb6=0;g_cfg.db_pb7=0;g_cfg.click_mode=0;g_cfg.debounce_scale=100;g_cfg.button_hold=0;
g_cfg.wheel_div=3;g_cfg.wheel_inv=0;g_cfg.wheel_smooth=1;g_cfg.wheel_smooth_strength=50;g_cfg.wheel_accel=0;g_cfg.wheel_noise=1;
g_cfg.hyprx_en=1;g_cfg.hyprx_led=1;g_cfg.hyprx_target=0;g_cfg.hyprx_stc=1;g_cfg.hyprx_div=4;g_cfg.hyprx_hold=0;g_cfg.hyprx_interval=0;g_cfg.hyprx_mode=0;
g_cfg.led_def[0]=0;g_cfg.led_def[1]=0;g_cfg.led_def[2]=255;g_cfg.led_macro[0]=255;g_cfg.led_macro[1]=255;g_cfg.led_macro[2]=0;
g_cfg.led_bri=255;g_cfg.led_eff=1;g_cfg.led_speed=100;g_cfg.led_idle=0;g_cfg.led_activity=1;
g_cfg.pio_mode=0;g_cfg.pio_trigger=0;g_cfg.pio_mouse=0;g_cfg.pio_key=0;g_cfg.pio_combo_len=0;g_cfg.pio_media=0;strncpy(g_cfg.pio_text,"Program Me!",55);
g_cfg.config_version=CONFIG_VERSION;
}
int config_reset_defaults(void){defaults();return 0;}
void config_init(void){crc_init();defaults();config_load();}
#define FLASH_CFG_ADDR 0x0801F800
#define FLASH_PAGE_SIZE 0x800
static int flash_erase(uint32_t addr){HAL_FLASH_Unlock();__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);FLASH_EraseInitTypeDef e={};e.TypeErase=FLASH_TYPEERASE_PAGES;e.Banks=FLASH_BANK_1;e.Page=(addr-0x08000000)/FLASH_PAGE_SIZE;e.NbPages=1;uint32_t err=0;__disable_irq();int r=HAL_FLASHEx_Erase(&e,&err)==HAL_OK?0:-1;__enable_irq();HAL_FLASH_Lock();return r;}
static int flash_write(uint32_t addr,uint8_t *data,uint32_t len){HAL_FLASH_Unlock();__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);int r=0;__disable_irq();for(uint32_t i=0;i<len&&!r;i+=8){uint64_t d=~0ull;for(uint32_t j=0;j<8&&i+j<len;j++){d&=~(0xFFull<<(j*8));d|=((uint64_t)data[i+j])<<(j*8);}if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,addr+i,d)!=HAL_OK)r=-1;}__enable_irq();HAL_FLASH_Lock();return r;}
int config_save(void){g_cfg.crc32=0;uint32_t crc=config_crc32((uint8_t*)&g_cfg,sizeof(g_cfg)-4);g_cfg.crc32=crc;if(flash_erase(FLASH_CFG_ADDR)!=0)return -1;if(flash_write(FLASH_CFG_ADDR,(uint8_t*)&g_cfg,sizeof(g_cfg))!=0)return -1;return 0;}
int config_load(void){config_t *p=(config_t*)FLASH_CFG_ADDR;if(p->config_version!=CONFIG_VERSION)return -1;uint32_t stored=p->crc32;config_t tmp;memcpy(&tmp,p,sizeof(tmp));tmp.crc32=0;uint32_t calc=config_crc32((uint8_t*)&tmp,sizeof(tmp)-4);if(calc!=stored)return -1;memcpy(&g_cfg,p,sizeof(g_cfg));return 0;}
static int u16set(uint8_t *d,uint16_t len,uint16_t lo,uint16_t hi,uint16_t *dst){if(len!=2)return -1;uint16_t v=d[0]|((uint16_t)d[1]<<8);if(v<lo||v>hi)return -1;*dst=v;return 0;}
static int u8set(uint8_t *d,uint16_t len,uint8_t lo,uint8_t hi,uint8_t *dst){if(len!=1||d[0]<lo||d[0]>hi)return -1;*dst=d[0];return 0;}
int config_set_field(uint8_t fid,uint8_t *data,uint16_t len){
if(!data||len>56)return -1;
switch(fid){
case 1:if(len>31)return -1;memset(g_cfg.device_name,0,32);memcpy(g_cfg.device_name,data,len);return 0;
case 2:if(len>31)return -1;memset(g_cfg.manufacturer,0,32);memcpy(g_cfg.manufacturer,data,len);return 0;
case 3:if(len>15)return -1;memset(g_cfg.serial,0,16);memcpy(g_cfg.serial,data,len);return 0;
case 22:return u16set(data,len,125,1000,&g_cfg.poll);
case 23:return u8set(data,len,0,1,&g_cfg.sof_sync);
case 32:return u16set(data,len,100,32000,&g_cfg.dpi);
case 33:{if(len!=2)return -1;uint16_t v=data[0]|((uint16_t)data[1]<<8);if(v!=1000&&v!=2000&&v!=4000&&v!=8000&&v!=12000&&v!=16000&&v!=24000)return -1;g_cfg.sensor_poll=v;return 0;}
case 34:return u8set(data,len,1,3,&g_cfg.lod_mm);
case 35:return u8set(data,len,10,200,&g_cfg.lod_squal);
case 36:return u8set(data,len,0,1,&g_cfg.angle_snap);
case 37:return u8set(data,len,0,100,&g_cfg.angle_strength);
case 38:return u8set(data,len,0,3,&g_cfg.surface);
case 39:return u16set(data,len,100,2000,&g_cfg.glitch_limit);
case 40:return u8set(data,len,0,100,&g_cfg.motion_smooth);
case 41:return u8set(data,len,1,24,&g_cfg.burst_window);
case 42:return u8set(data,len,0,100,&g_cfg.low_response);
case 43:return u8set(data,len,0,100,&g_cfg.high_response);
case 44:return u8set(data,len,0,20,&g_cfg.track_hyst);
case 48:return u8set(data,len,0,20,&g_cfg.db_pb3);
case 49:return u8set(data,len,0,20,&g_cfg.db_pb4);
case 50:return u8set(data,len,0,20,&g_cfg.db_pb5);
case 51:return u8set(data,len,0,20,&g_cfg.db_pb6);
case 52:return u8set(data,len,0,20,&g_cfg.db_pb7);
case 53:return u8set(data,len,0,2,&g_cfg.click_mode);
case 54:return u8set(data,len,50,200,&g_cfg.debounce_scale);
case 55:return u8set(data,len,0,10,&g_cfg.button_hold);
case 64:return u8set(data,len,1,8,&g_cfg.wheel_div);
case 65:return u8set(data,len,0,1,&g_cfg.wheel_inv);
case 66:return u8set(data,len,0,1,&g_cfg.wheel_smooth);
case 67:return u8set(data,len,0,100,&g_cfg.wheel_smooth_strength);
case 68:return u8set(data,len,0,100,&g_cfg.wheel_accel);
case 69:return u8set(data,len,0,3,&g_cfg.wheel_noise);
case 80:return u8set(data,len,0,1,&g_cfg.hyprx_en);
case 81:return u8set(data,len,0,1,&g_cfg.hyprx_led);
case 82:return u8set(data,len,0,1,&g_cfg.hyprx_target);
case 83:return u8set(data,len,1,24,&g_cfg.hyprx_stc);
case 84:return u8set(data,len,1,4,&g_cfg.hyprx_div);
case 85:return u16set(data,len,0,500,&g_cfg.hyprx_hold);
case 86:return u16set(data,len,0,100,&g_cfg.hyprx_interval);
case 87:return u8set(data,len,0,1,&g_cfg.hyprx_mode);
case 96:if(len!=3)return -1;memcpy(g_cfg.led_def,data,3);return 0;
case 97:if(len!=3)return -1;memcpy(g_cfg.led_macro,data,3);return 0;
case 98:return u8set(data,len,0,255,&g_cfg.led_bri);
case 99:return u8set(data,len,0,4,&g_cfg.led_eff);
case 100:return u8set(data,len,0,255,&g_cfg.led_speed);
case 101:return u16set(data,len,0,600,&g_cfg.led_idle);
case 102:return u8set(data,len,0,1,&g_cfg.led_activity);
case 112:return u8set(data,len,0,5,&g_cfg.pio_mode);
case 113:return u8set(data,len,0,3,&g_cfg.pio_trigger);
case 114:return u8set(data,len,0,7,&g_cfg.pio_mouse);
case 115:return u8set(data,len,0,255,&g_cfg.pio_key);
case 116:if(len>8)return -1;memset(g_cfg.pio_combo,0,8);memcpy(g_cfg.pio_combo,data,len);g_cfg.pio_combo_len=len;return 0;
case 117:return u8set(data,len,0,12,&g_cfg.pio_media);
case 118:if(len>55)return -1;memset(g_cfg.pio_text,0,56);memcpy(g_cfg.pio_text,data,len);return 0;
default:
if(fid>=FIELD_PIO_BLOCK0&&fid<=FIELD_PIO_BLOCK7){uint16_t off=(uint16_t)(fid-FIELD_PIO_BLOCK0)*PIO_BLOCK_SIZE;memset(g_cfg.pio_program+off,0,PIO_BLOCK_SIZE);memcpy(g_cfg.pio_program+off,data,len);return 0;}
return -1;
}}
static int getstr(const char *s,uint16_t max,uint8_t *out,uint16_t *len){uint16_t n=strnlen(s,max);memcpy(out,s,n);*len=n;return 0;}
static int getu16(uint16_t v,uint8_t *out,uint16_t *len){out[0]=v&255;out[1]=v>>8;*len=2;return 0;}
int config_get_field(uint8_t fid,uint8_t *out,uint16_t *out_len){
if(!out||!out_len)return -1;
switch(fid){
case 1:return getstr(g_cfg.device_name,32,out,out_len);
case 2:return getstr(g_cfg.manufacturer,32,out,out_len);
case 3:return getstr(g_cfg.serial,16,out,out_len);
case 4:return getstr(g_cfg.fw_version,16,out,out_len);
case 22:return getu16(g_cfg.poll,out,out_len);
case 23:*out=g_cfg.sof_sync;*out_len=1;return 0;
case 32:return getu16(g_cfg.dpi,out,out_len);
case 33:return getu16(g_cfg.sensor_poll,out,out_len);
case 34:*out=g_cfg.lod_mm;*out_len=1;return 0;
case 35:*out=g_cfg.lod_squal;*out_len=1;return 0;
case 36:*out=g_cfg.angle_snap;*out_len=1;return 0;
case 37:*out=g_cfg.angle_strength;*out_len=1;return 0;
case 38:*out=g_cfg.surface;*out_len=1;return 0;
case 39:return getu16(g_cfg.glitch_limit,out,out_len);
case 40:*out=g_cfg.motion_smooth;*out_len=1;return 0;
case 41:*out=g_cfg.burst_window;*out_len=1;return 0;
case 42:*out=g_cfg.low_response;*out_len=1;return 0;
case 43:*out=g_cfg.high_response;*out_len=1;return 0;
case 44:*out=g_cfg.track_hyst;*out_len=1;return 0;
case 48:*out=g_cfg.db_pb3;*out_len=1;return 0;
case 49:*out=g_cfg.db_pb4;*out_len=1;return 0;
case 50:*out=g_cfg.db_pb5;*out_len=1;return 0;
case 51:*out=g_cfg.db_pb6;*out_len=1;return 0;
case 52:*out=g_cfg.db_pb7;*out_len=1;return 0;
case 53:*out=g_cfg.click_mode;*out_len=1;return 0;
case 54:*out=g_cfg.debounce_scale;*out_len=1;return 0;
case 55:*out=g_cfg.button_hold;*out_len=1;return 0;
case 64:*out=g_cfg.wheel_div;*out_len=1;return 0;
case 65:*out=g_cfg.wheel_inv;*out_len=1;return 0;
case 66:*out=g_cfg.wheel_smooth;*out_len=1;return 0;
case 67:*out=g_cfg.wheel_smooth_strength;*out_len=1;return 0;
case 68:*out=g_cfg.wheel_accel;*out_len=1;return 0;
case 69:*out=g_cfg.wheel_noise;*out_len=1;return 0;
case 80:*out=g_cfg.hyprx_en;*out_len=1;return 0;
case 81:*out=g_cfg.hyprx_led;*out_len=1;return 0;
case 82:*out=g_cfg.hyprx_target;*out_len=1;return 0;
case 83:*out=g_cfg.hyprx_stc;*out_len=1;return 0;
case 84:*out=g_cfg.hyprx_div;*out_len=1;return 0;
case 85:return getu16(g_cfg.hyprx_hold,out,out_len);
case 86:return getu16(g_cfg.hyprx_interval,out,out_len);
case 87:*out=g_cfg.hyprx_mode;*out_len=1;return 0;
case 96:memcpy(out,g_cfg.led_def,3);*out_len=3;return 0;
case 97:memcpy(out,g_cfg.led_macro,3);*out_len=3;return 0;
case 98:*out=g_cfg.led_bri;*out_len=1;return 0;
case 99:*out=g_cfg.led_eff;*out_len=1;return 0;
case 100:*out=g_cfg.led_speed;*out_len=1;return 0;
case 101:return getu16(g_cfg.led_idle,out,out_len);
case 102:*out=g_cfg.led_activity;*out_len=1;return 0;
case 112:*out=g_cfg.pio_mode;*out_len=1;return 0;
case 113:*out=g_cfg.pio_trigger;*out_len=1;return 0;
case 114:*out=g_cfg.pio_mouse;*out_len=1;return 0;
case 115:*out=g_cfg.pio_key;*out_len=1;return 0;
case 116:memcpy(out,g_cfg.pio_combo,g_cfg.pio_combo_len);*out_len=g_cfg.pio_combo_len;return 0;
case 117:*out=g_cfg.pio_media;*out_len=1;return 0;
case 118:return getstr(g_cfg.pio_text,56,out,out_len);
default:
if(fid>=FIELD_PIO_BLOCK0&&fid<=FIELD_PIO_BLOCK7){uint16_t off=(uint16_t)(fid-FIELD_PIO_BLOCK0)*PIO_BLOCK_SIZE;memcpy(out,g_cfg.pio_program+off,PIO_BLOCK_SIZE);*out_len=PIO_BLOCK_SIZE;return 0;}
return -1;
}}
