#include "config.h"
#include <string.h>
#include <stm32g4xx_hal.h>
config_t g_cfg;
static uint32_t table[256];
static void crc_init(void){
for(uint32_t i=0;i<256;i++){
uint32_t c=i;
for(int j=0;j<8;j++)c=(c&1)?(0xEDB88320u^(c>>1)):(c>>1);
table[i]=c;
}
}
uint32_t config_crc32(uint8_t *d,uint32_t l){
uint32_t c=0xFFFFFFFFu;
for(uint32_t i=0;i<l;i++)c=table[(c^d[i])&0xFF]^(c>>8);
return c^0xFFFFFFFFu;
}
static void defaults(void){
memset(&g_cfg,0,sizeof(g_cfg));
strncpy(g_cfg.device_name,"Cobalt-X Zero",31);
strncpy(g_cfg.manufacturer,"Synarix",31);
strncpy(g_cfg.serial,"CXZ-000001",15);
strncpy(g_cfg.fw_version,"1.0.0",15);
g_cfg.vid=0x1209;
g_cfg.pid=0xC0BA;
g_cfg.mod_sensor=1;
g_cfg.mod_buttons=1;
g_cfg.mod_scroll=1;
g_cfg.mod_hyprx=1;
g_cfg.mod_leds=1;
g_cfg.mod_sof=1;
g_cfg.dpi=800;
g_cfg.poll=1000;
g_cfg.lod_mm=2;
g_cfg.lod_squal=20;
g_cfg.angle_snap=0;
g_cfg.angle_strength=0;
g_cfg.surface=0;
g_cfg.db_pb3=0;
g_cfg.db_pb4=0;
g_cfg.db_pb5=0;
g_cfg.db_pb6=0;
g_cfg.db_pb7=0;
g_cfg.click_mode=0;
g_cfg.wheel_div=3;
g_cfg.wheel_inv=0;
g_cfg.wheel_smooth=1;
g_cfg.hyprx_en=1;
g_cfg.hyprx_led=1;
g_cfg.hyprx_target=0;
g_cfg.hyprx_stc=1;
g_cfg.hyprx_div=4;
g_cfg.hyprx_hold=0;
g_cfg.led_def[0]=0x00;
g_cfg.led_def[1]=0x00;
g_cfg.led_def[2]=0xFF;
g_cfg.led_macro[0]=0xFF;
g_cfg.led_macro[1]=0xFF;
g_cfg.led_macro[2]=0x00;
g_cfg.led_bri=255;
g_cfg.led_eff=1;
g_cfg.led_speed=100;
g_cfg.pb7_mode=0;
g_cfg.pb7_at=0;
g_cfg.pb7_mouse=0;
g_cfg.pb7_key=0;
g_cfg.pb7_combo_len=0;
g_cfg.pb7_media=0;
g_cfg.config_version=CONFIG_VERSION;
}
int config_reset_defaults(void){defaults();return 0;}
void config_init(void){crc_init();defaults();config_load();}
#define FLASH_CFG_ADDR 0x0801F800
#define FLASH_PAGE_SIZE 0x800
static int flash_erase(uint32_t addr){
HAL_FLASH_Unlock();
__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
FLASH_EraseInitTypeDef e={};
e.TypeErase=FLASH_TYPEERASE_PAGES;
e.Banks=FLASH_BANK_1;
e.Page=(addr-0x08000000)/FLASH_PAGE_SIZE;
e.NbPages=1;
uint32_t err=0;
__disable_irq();
int r=HAL_FLASHEx_Erase(&e,&err)==HAL_OK?0:-1;
__enable_irq();
HAL_FLASH_Lock();
return r;
}
static int flash_write(uint32_t addr,uint8_t *data,uint32_t len){
HAL_FLASH_Unlock();
__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
int r=0;
__disable_irq();
for(uint32_t i=0;i<len&&!r;i+=8){
uint64_t d=~0ull;
for(uint32_t j=0;j<8&&i+j<len;j++){
d&=~(0xFFull<<(j*8));
d|=((uint64_t)data[i+j])<<(j*8);
}
if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,addr+i,d)!=HAL_OK)r=-1;
}
__enable_irq();
HAL_FLASH_Lock();
return r;
}
int config_save(void){
g_cfg.crc32=0;
uint32_t crc=config_crc32((uint8_t*)&g_cfg,sizeof(g_cfg)-4);
g_cfg.crc32=crc;
if(flash_erase(FLASH_CFG_ADDR)!=0)return -1;
if(flash_write(FLASH_CFG_ADDR,(uint8_t*)&g_cfg,sizeof(g_cfg))!=0)return -1;
return 0;
}
int config_load(void){
config_t *p=(config_t*)FLASH_CFG_ADDR;
if(p->config_version!=CONFIG_VERSION)return -1;
uint32_t stored=p->crc32;
config_t tmp;
memcpy(&tmp,p,sizeof(tmp));
tmp.crc32=0;
uint32_t calc=config_crc32((uint8_t*)&tmp,sizeof(tmp)-4);
if(calc!=stored)return -1;
memcpy(&g_cfg,p,sizeof(g_cfg));
return 0;
}
int config_set_field(uint8_t fid,uint8_t *data,uint16_t len){
if(!data||len>60)return -1;
switch(fid){
case 1:{if(len>31)return -1;memset(g_cfg.device_name,0,32);memcpy(g_cfg.device_name,data,len);return 0;}
case 2:{if(len>31)return -1;memset(g_cfg.manufacturer,0,32);memcpy(g_cfg.manufacturer,data,len);return 0;}
case 3:{if(len>15)return -1;memset(g_cfg.serial,0,16);memcpy(g_cfg.serial,data,len);return 0;}
case 4:{if(len>15)return -1;memset(g_cfg.fw_version,0,16);memcpy(g_cfg.fw_version,data,len);return 0;}
case 16:if(len!=1||data[0]>1)return -1;g_cfg.mod_sensor=data[0];return 0;
case 17:if(len!=1||data[0]>1)return -1;g_cfg.mod_buttons=data[0];return 0;
case 18:if(len!=1||data[0]>1)return -1;g_cfg.mod_scroll=data[0];return 0;
case 19:if(len!=1||data[0]>1)return -1;g_cfg.mod_hyprx=data[0];return 0;
case 20:if(len!=1||data[0]>1)return -1;g_cfg.mod_leds=data[0];return 0;
case 21:if(len!=1||data[0]>1)return -1;g_cfg.mod_sof=data[0];return 0;
case 32:{if(len!=2)return -1;uint16_t v=data[0]|(data[1]<<8);if(v<100||v>32000)return -1;g_cfg.dpi=v;return 0;}
case 33:{if(len!=2)return -1;uint16_t v=data[0]|(data[1]<<8);if(v!=125&&v!=250&&v!=500&&v!=1000&&v!=2000&&v!=4000&&v!=8000)return -1;g_cfg.poll=v;return 0;}
case 34:if(len!=1||data[0]<1||data[0]>3)return -1;g_cfg.lod_mm=data[0];return 0;
case 35:if(len!=1||data[0]<10||data[0]>200)return -1;g_cfg.lod_squal=data[0];return 0;
case 36:if(len!=1||data[0]>1)return -1;g_cfg.angle_snap=data[0];return 0;
case 37:if(len!=1||data[0]>100)return -1;g_cfg.angle_strength=data[0];return 0;
case 38:if(len!=1||data[0]>3)return -1;g_cfg.surface=data[0];return 0;
case 48:if(len!=1||data[0]>20)return -1;g_cfg.db_pb3=data[0];return 0;
case 49:if(len!=1||data[0]>20)return -1;g_cfg.db_pb4=data[0];return 0;
case 50:if(len!=1||data[0]>20)return -1;g_cfg.db_pb5=data[0];return 0;
case 51:if(len!=1||data[0]>20)return -1;g_cfg.db_pb6=data[0];return 0;
case 52:if(len!=1||data[0]>20)return -1;g_cfg.db_pb7=data[0];return 0;
case 53:if(len!=1||data[0]>2)return -1;g_cfg.click_mode=data[0];return 0;
case 64:if(len!=1||data[0]<1||data[0]>8)return -1;g_cfg.wheel_div=data[0];return 0;
case 65:if(len!=1||data[0]>1)return -1;g_cfg.wheel_inv=data[0];return 0;
case 66:if(len!=1||data[0]>1)return -1;g_cfg.wheel_smooth=data[0];return 0;
case 80:if(len!=1||data[0]>1)return -1;g_cfg.hyprx_en=data[0];return 0;
case 81:if(len!=1||data[0]>1)return -1;g_cfg.hyprx_led=data[0];return 0;
case 82:if(len!=1||data[0]>1)return -1;g_cfg.hyprx_target=data[0];return 0;
case 83:if(len!=1||data[0]<1||data[0]>24)return -1;g_cfg.hyprx_stc=data[0];return 0;
case 84:if(len!=1||data[0]<1||data[0]>4)return -1;g_cfg.hyprx_div=data[0];return 0;
case 85:{if(len!=2)return -1;uint16_t v=data[0]|(data[1]<<8);if(v>500)return -1;g_cfg.hyprx_hold=v;return 0;}
case 96:if(len!=3)return -1;g_cfg.led_def[0]=data[0];g_cfg.led_def[1]=data[1];g_cfg.led_def[2]=data[2];return 0;
case 97:if(len!=3)return -1;g_cfg.led_macro[0]=data[0];g_cfg.led_macro[1]=data[1];g_cfg.led_macro[2]=data[2];return 0;
case 98:if(len!=1)return -1;g_cfg.led_bri=data[0];return 0;
case 99:if(len!=1||data[0]>4)return -1;g_cfg.led_eff=data[0];return 0;
case 100:if(len!=1)return -1;g_cfg.led_speed=data[0];return 0;
case 112:if(len!=1||data[0]>4)return -1;g_cfg.pb7_mode=data[0];return 0;
case 113:if(len!=1||data[0]>3)return -1;g_cfg.pb7_at=data[0];return 0;
case 114:if(len!=1||data[0]>7)return -1;g_cfg.pb7_mouse=data[0];return 0;
case 115:if(len!=1)return -1;g_cfg.pb7_key=data[0];return 0;
case 116:if(len>8)return -1;memset(g_cfg.pb7_combo,0,8);memcpy(g_cfg.pb7_combo,data,len);g_cfg.pb7_combo_len=len;return 0;
case 117:if(len!=1||data[0]>12)return -1;g_cfg.pb7_media=data[0];return 0;
default:return -1;
}
}
int config_get_field(uint8_t fid,uint8_t *out,uint16_t *out_len){
if(!out||!out_len)return -1;
switch(fid){
case 1:{uint16_t l=strnlen(g_cfg.device_name,32);memcpy(out,g_cfg.device_name,l);*out_len=l;return 0;}
case 2:{uint16_t l=strnlen(g_cfg.manufacturer,32);memcpy(out,g_cfg.manufacturer,l);*out_len=l;return 0;}
case 3:{uint16_t l=strnlen(g_cfg.serial,16);memcpy(out,g_cfg.serial,l);*out_len=l;return 0;}
case 4:{uint16_t l=strnlen(g_cfg.fw_version,16);memcpy(out,g_cfg.fw_version,l);*out_len=l;return 0;}
case 16:out[0]=g_cfg.mod_sensor;*out_len=1;return 0;
case 17:out[0]=g_cfg.mod_buttons;*out_len=1;return 0;
case 18:out[0]=g_cfg.mod_scroll;*out_len=1;return 0;
case 19:out[0]=g_cfg.mod_hyprx;*out_len=1;return 0;
case 20:out[0]=g_cfg.mod_leds;*out_len=1;return 0;
case 21:out[0]=g_cfg.mod_sof;*out_len=1;return 0;
case 32:out[0]=g_cfg.dpi&0xFF;out[1]=g_cfg.dpi>>8;*out_len=2;return 0;
case 33:out[0]=g_cfg.poll&0xFF;out[1]=g_cfg.poll>>8;*out_len=2;return 0;
case 34:out[0]=g_cfg.lod_mm;*out_len=1;return 0;
case 35:out[0]=g_cfg.lod_squal;*out_len=1;return 0;
case 36:out[0]=g_cfg.angle_snap;*out_len=1;return 0;
case 37:out[0]=g_cfg.angle_strength;*out_len=1;return 0;
case 38:out[0]=g_cfg.surface;*out_len=1;return 0;
case 48:out[0]=g_cfg.db_pb3;*out_len=1;return 0;
case 49:out[0]=g_cfg.db_pb4;*out_len=1;return 0;
case 50:out[0]=g_cfg.db_pb5;*out_len=1;return 0;
case 51:out[0]=g_cfg.db_pb6;*out_len=1;return 0;
case 52:out[0]=g_cfg.db_pb7;*out_len=1;return 0;
case 53:out[0]=g_cfg.click_mode;*out_len=1;return 0;
case 64:out[0]=g_cfg.wheel_div;*out_len=1;return 0;
case 65:out[0]=g_cfg.wheel_inv;*out_len=1;return 0;
case 66:out[0]=g_cfg.wheel_smooth;*out_len=1;return 0;
case 80:out[0]=g_cfg.hyprx_en;*out_len=1;return 0;
case 81:out[0]=g_cfg.hyprx_led;*out_len=1;return 0;
case 82:out[0]=g_cfg.hyprx_target;*out_len=1;return 0;
case 83:out[0]=g_cfg.hyprx_stc;*out_len=1;return 0;
case 84:out[0]=g_cfg.hyprx_div;*out_len=1;return 0;
case 85:out[0]=g_cfg.hyprx_hold&0xFF;out[1]=g_cfg.hyprx_hold>>8;*out_len=2;return 0;
case 96:out[0]=g_cfg.led_def[0];out[1]=g_cfg.led_def[1];out[2]=g_cfg.led_def[2];*out_len=3;return 0;
case 97:out[0]=g_cfg.led_macro[0];out[1]=g_cfg.led_macro[1];out[2]=g_cfg.led_macro[2];*out_len=3;return 0;
case 98:out[0]=g_cfg.led_bri;*out_len=1;return 0;
case 99:out[0]=g_cfg.led_eff;*out_len=1;return 0;
case 100:out[0]=g_cfg.led_speed;*out_len=1;return 0;
case 112:out[0]=g_cfg.pb7_mode;*out_len=1;return 0;
case 113:out[0]=g_cfg.pb7_at;*out_len=1;return 0;
case 114:out[0]=g_cfg.pb7_mouse;*out_len=1;return 0;
case 115:out[0]=g_cfg.pb7_key;*out_len=1;return 0;
case 116:memcpy(out,g_cfg.pb7_combo,g_cfg.pb7_combo_len);*out_len=g_cfg.pb7_combo_len;return 0;
case 117:out[0]=g_cfg.pb7_media;*out_len=1;return 0;
default:return -1;
}
}
