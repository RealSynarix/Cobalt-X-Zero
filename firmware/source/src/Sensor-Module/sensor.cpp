#include "sensor.h"
#include "sensor_filter.h"
#include "srom_04.h"
#include "../Config-Module/config.h"
#include "../USB-Module/device.h"
#include <Arduino.h>
#include <SPI.h>
#include <stm32g4xx.h>
#define PIN_CS PA4
#define PIN_RST PA2
#define REG_Product_ID 0x00
#define REG_Motion 0x02
#define REG_Config1 0x0F
#define REG_Config2 0x10
#define REG_SROM_Enable 0x13
#define REG_SROM_ID 0x2A
#define REG_Power_Up_Reset 0x3A
#define REG_Motion_Burst 0x50
#define REG_SROM_Load_Burst 0x62
#define REG_Lift_Config 0x63
#define SF_N 8
#define SENSOR_HZ 8000u
#define BURST_LEN 7
#define LOD1_MARGIN 5
#define PEND_LIMIT 2032
#define SPI_SET SPISettings(2000000,MSBFIRST,SPI_MODE3)
static uint8_t ready=0;
static uint8_t idle=1;
static uint8_t locked=0;
static uint16_t last_fn=0xFFFF;
static uint32_t last_sof=0,t_next=0;
static sf_packet_t pk[SF_N];
static uint8_t np=0;
static int32_t pend_x=0,pend_y=0;
static float rem_x=0.0f,rem_y=0.0f;
static uint16_t ap_dpi=0,ap_cpi=800;
static uint8_t ap_lod=0;
static inline uint32_t cyc(void){return DWT->CYCCNT;}
static void dly_us(uint32_t us){uint32_t t=cyc(),n=us*(SystemCoreClock/1000000u);while((cyc()-t)<n){}}
static inline void cs_low(void){digitalWrite(PIN_CS,LOW);}
static inline void cs_high(void){digitalWrite(PIN_CS,HIGH);}
static uint8_t rreg(uint8_t a){
SPI.beginTransaction(SPI_SET);
cs_low();
SPI.transfer(a&0x7F);
dly_us(160);
uint8_t d=SPI.transfer(0);
dly_us(1);
cs_high();
SPI.endTransaction();
dly_us(30);
return d;
}
static void wreg(uint8_t a,uint8_t d){
SPI.beginTransaction(SPI_SET);
cs_low();
SPI.transfer(a|0x80);
SPI.transfer(d);
dly_us(35);
cs_high();
SPI.endTransaction();
dly_us(150);
}
static void apply_cfg(void){
uint16_t v=g_cfg.dpi/100u;
if(v<1)v=1;
if(v>120)v=120;
wreg(REG_Config1,(uint8_t)(v-1));
wreg(REG_Lift_Config,g_cfg.lod_mm>=3?0x03:0x02);
wreg(REG_Motion_Burst,0x00);
ap_dpi=g_cfg.dpi;
ap_lod=g_cfg.lod_mm;
ap_cpi=(uint16_t)(v*100u);
}
static void read_packet(sf_packet_t *q){
uint8_t d[BURST_LEN]={0};
SPI.beginTransaction(SPI_SET);
cs_low();
SPI.transfer(REG_Motion_Burst);
dly_us(36);
SPI.transfer(d,BURST_LEN);
cs_high();
SPI.endTransaction();
q->lifted=(d[0]&0x08)?1:0;
q->squal=d[6];
if(d[0]&0x80){
int16_t rx=(int16_t)((d[3]<<8)|d[2]);
int16_t ry=(int16_t)((d[5]<<8)|d[4]);
q->dx=(int16_t)-rx;
q->dy=(int16_t)-ry;
}else{
q->dx=0;q->dy=0;
}
}
static void flush(void){
sf_packet_t t;
read_packet(&t);
dly_us(130);
read_packet(&t);
sf_reset();
np=0;
}
static uint8_t try_init(void){
cs_high();
wreg(REG_Power_Up_Reset,0x5A);
delay(80);
for(uint8_t r=0x02;r<=0x06;r++)rreg(r);
delay(10);
if(rreg(REG_Product_ID)!=0x42)return 0;
wreg(REG_Config2,0x00);
delay(10);
wreg(REG_SROM_Enable,0x1D);
delay(10);
wreg(REG_SROM_Enable,0x18);
delay(10);
SPI.beginTransaction(SPI_SET);
cs_low();
SPI.transfer(REG_SROM_Load_Burst|0x80);
dly_us(15);
for(uint16_t i=0;i<SROM_04_LENGTH;i++){
SPI.transfer(SROM_04[i]);
dly_us(15);
}
cs_high();
SPI.endTransaction();
delay(30);
if(rreg(REG_SROM_ID)!=0x04)return 0;
ap_dpi=0;ap_lod=0;
apply_cfg();
wreg(REG_Config2,0x00);
delay(30);
rreg(REG_Motion);
delay(10);
wreg(REG_Motion_Burst,0x00);
return 1;
}
extern "C" {
void sensor_init(void){
ready=0;
pinMode(PIN_RST,OUTPUT);
digitalWrite(PIN_RST,HIGH);
delay(200);
pinMode(PIN_CS,OUTPUT);
cs_high();
delay(100);
SPI.setMISO(PA6);
SPI.setMOSI(PA7);
SPI.setSCLK(PA5);
SPI.begin();
delay(200);
for(uint8_t i=0;i<3&&!ready;i++)ready=try_init();
sf_reset();
idle=1;locked=0;np=0;last_fn=0xFFFF;
pend_x=pend_y=0;rem_x=rem_y=0.0f;
}
uint8_t sensor_ready(void){return ready;}
static void close_frame(void){
sf_params_t prm;
uint16_t sq=g_cfg.lod_squal;
if(g_cfg.lod_mm==1)sq+=LOD1_MARGIN;
prm.squal_min=(uint8_t)(sq>255?255:sq);
prm.dpi=ap_cpi;
prm.snap_en=g_cfg.angle_snap;
prm.snap_strength=g_cfg.angle_strength;
switch(g_cfg.surface){
case 1:prm.a_min=0.25f;break;
case 2:prm.a_min=0.35f;break;
default:prm.a_min=0.30f;break;
}
float ox,oy;
sf_frame(pk,np,&prm,&ox,&oy);
rem_x+=ox;rem_y+=oy;
int32_t ix=(int32_t)rem_x,iy=(int32_t)rem_y;
rem_x-=(float)ix;rem_y-=(float)iy;
pend_x+=ix;pend_y+=iy;
if(pend_x>PEND_LIMIT)pend_x=PEND_LIMIT;
if(pend_x<-PEND_LIMIT)pend_x=-PEND_LIMIT;
if(pend_y>PEND_LIMIT)pend_y=PEND_LIMIT;
if(pend_y<-PEND_LIMIT)pend_y=-PEND_LIMIT;
}
void sensor_task(void){
if(!ready)return;
uint32_t now=cyc();
if(!g_cfg.mod_sensor||!usb_device_configured()){
idle=1;np=0;locked=0;
pend_x=pend_y=0;rem_x=rem_y=0.0f;
return;
}
if(idle){
idle=0;
if(g_cfg.dpi!=ap_dpi||g_cfg.lod_mm!=ap_lod)apply_cfg();
flush();
now=cyc();t_next=now;last_fn=0xFFFF;
}else if(g_cfg.dpi!=ap_dpi||g_cfg.lod_mm!=ap_lod){
apply_cfg();
flush();
now=cyc();t_next=now;
}
uint16_t fn=USB->FNR&USB_FNR_FN;
if(fn!=last_fn){
last_fn=fn;
last_sof=now;
if(locked)close_frame();
np=0;t_next=now;locked=1;
}else if(locked&&(now-last_sof)>(SystemCoreClock/1000u)*3u){
locked=0;
}
if(np<SF_N&&(int32_t)(now-t_next)>=0){
read_packet(&pk[np++]);
t_next+=SystemCoreClock/SENSOR_HZ;
uint32_t n2=cyc();
if((int32_t)(n2-t_next)>0)t_next=n2;
}
if(!locked&&np>=SF_N){close_frame();np=0;}
}
void sensor_take(int16_t *dx,int16_t *dy){
int32_t x=pend_x,y=pend_y;
int32_t m=(x<0?-x:x);
int32_t my=(y<0?-y:y);
if(my>m)m=my;
if(m>127){
x=(x*127)/m;
y=(y*127)/m;
}
pend_x-=x;pend_y-=y;
*dx=(int16_t)x;*dy=(int16_t)y;
}
}