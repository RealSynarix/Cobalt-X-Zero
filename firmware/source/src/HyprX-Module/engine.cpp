#include "engine.h"
#include "../Config-Module/config.h"
#include <stm32g4xx.h>
#define PEND_MAX 512
static uint8_t mode=0,target=0,phase=0;
static uint8_t last_mac=0,last_mmb=0,last_lmb=0,last_rmb=0;
static int32_t pending=0,acc=0,stepacc=0;
static uint32_t stamp=0;
static void clear(void){pending=0;acc=0;stepacc=0;phase=0;}
void hyprx_init(void){mode=0;target=0;last_mac=0;last_mmb=0;last_lmb=0;last_rmb=0;stamp=0;clear();}
uint8_t hyprx_update(uint8_t b,int32_t *d){
uint8_t lmb=b&1u,rmb=(b>>1)&1u,mmb=(b>>2)&1u,mac=(b>>3)&1u;
uint8_t armed=g_cfg.mod_hyprx&&g_cfg.hyprx_en;
if(!armed&&mode){mode=0;clear();}
if(armed&&mac&&!last_mac){mode^=1u;target=g_cfg.hyprx_target;clear();}
if(mode){
if(mmb){if(lmb&&!last_lmb)target=0;if(rmb&&!last_rmb)target=1;}
if(mmb&&!last_mmb){if(lmb&&!rmb)target=0;else if(rmb&&!lmb)target=1;else if(lmb&&rmb)target=0;}
}
last_mac=mac;last_mmb=mmb;last_lmb=lmb;last_rmb=rmb;
if(!mode||*d==0){uint8_t base=b;if(mode)base&=~0x04u;return base&0x0Fu;}
int32_t dv=g_cfg.hyprx_div?*d/(int32_t)g_cfg.hyprx_div:*d;
pending+=dv;
int32_t st=g_cfg.hyprx_stc;
if(st<1)st=1;
if(pending>=st||pending<=-st){
acc+=pending>0?1:-1;
stepacc+=pending>0?1:-1;
pending=0;
stamp=DWT->CYCCNT;
}
if(acc!=0){
uint32_t now=DWT->CYCCNT;
uint32_t hold=g_cfg.hyprx_hold*(SystemCoreClock/1000u);
if((now-stamp)>=hold){
int32_t out=acc>0?1:-1;
acc-=out;
*d=out;
uint8_t base=b;
if(target==0)base|=0x01u;
else base|=0x02u;
return base&0x0Fu;
}
}
*d=0;
return b&0x0Fu;
}
uint8_t hyprx_tick(void){return mode;}
uint8_t hyprx_active(void){return mode;}
