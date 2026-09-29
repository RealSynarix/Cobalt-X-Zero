#include "tactile.h"
#include "../HyprX-Module/engine.h"
#include "../Config-Module/config.h"
#include <stm32g4xx.h>
#define BASE_LOCK 34000u
#define FRAME_DIV 32
#define QSIZE 32
#define QMASK 31
typedef struct{uint8_t b;int8_t w;uint8_t m;}qitem_t;
static qitem_t q[QSIZE];
static volatile uint8_t qh=0,qt=0,qc=0;
static volatile uint8_t g_buttons=0;
static uint8_t phys_state=0;
static uint32_t last_cyc[5]={0,0,0,0,0};
static int32_t wheel_acc=0;
static uint32_t last_enc=0;
static uint8_t frame=0;
static volatile uint8_t macro_q=0;
static inline void qpush(uint8_t b,int8_t w,uint8_t m){
uint32_t p=__get_PRIMASK();__disable_irq();
if(qc<QSIZE){q[qh].b=b;q[qh].w=w;q[qh].m=m;qh=(qh+1)&QMASK;qc++;}
if(!p)__enable_irq();
}
static inline uint32_t lockout(uint8_t i){
uint32_t ms;
switch(i){
case 0:ms=g_cfg.db_pb3;break;
case 1:ms=g_cfg.db_pb4;break;
case 2:ms=g_cfg.db_pb5;break;
case 3:ms=g_cfg.db_pb6;break;
default:ms=g_cfg.db_pb7;
}
ms*=SystemCoreClock/1000u;
switch(g_cfg.click_mode){
case 1:return ms*2u;
case 2:return ms/2u;
default:return ms;
}
}
static inline uint8_t read_pins(void){
uint32_t idr=GPIOB->IDR;
uint8_t s=0;
if(!(idr&(1u<<3)))s|=1u<<0;
if(!(idr&(1u<<4)))s|=1u<<1;
if(!(idr&(1u<<5)))s|=1u<<2;
if(!(idr&(1u<<6)))s|=1u<<3;
if(!(idr&(1u<<7)))s|=1u<<4;
return s;
}
static inline int8_t read_wheel(void){
uint32_t a=(GPIOB->IDR>>8)&1u;
uint32_t b=(GPIOB->IDR>>9)&1u;
uint32_t cur=(a<<1)|b;
int8_t dir=0;
if(last_enc==0){
if(cur==1)dir=1;
else if(cur==2)dir=-1;
}else if(last_enc==1){
if(cur==3)dir=1;
else if(cur==0)dir=-1;
}else if(last_enc==3){
if(cur==2)dir=1;
else if(cur==1)dir=-1;
}else if(last_enc==2){
if(cur==0)dir=1;
else if(cur==3)dir=-1;
}
last_enc=cur;
return dir;
}
extern "C" {
void tactile_init(void){
RCC->AHB2ENR|=RCC_AHB2ENR_GPIOBEN;__DSB();
GPIOB->MODER&=~((0x3u<<6)|(0x3u<<8)|(0x3u<<10)|(0x3u<<12)|(0x3u<<14)|(0x3u<<16)|(0x3u<<18));
GPIOB->PUPDR=(GPIOB->PUPDR&~((0x3u<<6)|(0x3u<<8)|(0x3u<<10)|(0x3u<<12)|(0x3u<<14)|(0x3u<<16)|(0x3u<<18)))|((1u<<6)|(1u<<8)|(1u<<10)|(1u<<12)|(1u<<14)|(1u<<16)|(1u<<18));
GPIOB->IDR;
last_enc=(GPIOB->IDR>>8)&3u;
phys_state=read_pins();
g_buttons=0;
qh=qt=qc=0;
}
void tactile_tick(void){
frame++;
uint8_t now=read_pins();
uint32_t cyc=DWT->CYCCNT;
for(uint8_t i=0;i<5;i++){
uint8_t mask=1u<<i;
uint8_t cur=now&mask;
uint8_t prev=phys_state&mask;
if(cur!=prev){
if((cyc-last_cyc[i])>=lockout(i)){
phys_state=(phys_state&~mask)|cur;
last_cyc[i]=cyc;
if(cur){
if(i<4)g_buttons|=mask;
else macro_q=1;
}else{
if(i<4)g_buttons&=~mask;
}
}
}
}
int8_t wd=read_wheel();
if(wd){
wheel_acc+=wd;
int32_t div=g_cfg.wheel_div?g_cfg.wheel_div:3;
if(wheel_acc>=div||wheel_acc<=-div){
int8_t w=wheel_acc>0?1:-1;
if(g_cfg.wheel_inv)w=-w;
qpush(g_buttons,w,0);
wheel_acc=0;
}
}
if((frame%FRAME_DIV)==0){
uint8_t b=g_buttons;
int32_t d=0;
b=hyprx_update(b,&d);
qpush(b,0,macro_q);
macro_q=0;
}
}
uint8_t tactile_pop(tactile_report_t *out){
uint32_t p=__get_PRIMASK();__disable_irq();
if(qc==0){if(!p)__enable_irq();return 0;}
qitem_t it=q[qt];
qt=(qt+1)&QMASK;
qc--;
if(!p)__enable_irq();
out->buttons=it.b;
out->wheel=it.w;
out->macro=it.m;
return 1;
}
}
