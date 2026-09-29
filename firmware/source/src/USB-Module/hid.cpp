#include "hid.h"
#include "device.h"
#include "../Config-Module/config.h"
#include "../Config-Module/webhid.h"
#include <string.h>
#include <Arduino.h>
#define REPORT_ID 2
#define KQ_SIZE 64
#define KQ_MASK 63
#define CLICK_FRAMES 20
#define PIO_MAGIC 0xC7
#define PIO_VERSION 1
uint8_t feature_report_buf[65];
static uint8_t req[64];
static volatile uint16_t reql=0;
static volatile uint8_t pend=0;
typedef struct{uint8_t n;uint8_t d[9];}krep_t;
static krep_t kq[KQ_SIZE];
static uint8_t kqh=0,kqt=0,kqc=0;
static uint8_t mb_mask=0,mb_frames=0,slot_cnt=0;
static uint8_t pio_mouse_mask=0,pio_click_mask=0,pio_click_frames=0;
static int16_t pio_dx=0,pio_dy=0;
static int16_t pio_wheel=0;
static uint8_t pio_pending=0,pio_active=0;
static uint16_t pio_pos=0,pio_end=0;
static uint32_t pio_wait=0;
static uint8_t pio_text_active=0,pio_text_pos=0,pio_text_len=0;
static const char *pio_text_ptr=nullptr;
static uint8_t pio_kmods=0,pio_kkeys[6]={0,0,0,0,0,0};
static uint8_t pio_kcount=0;
static uint32_t last_activity=0;
static const uint16_t media_map[13]={0x00CD,0x00B5,0x00B6,0x00B7,0x00E2,0x00E9,0x00EA,0x0192,0x0223,0x018A,0x0183,0x006F,0x0070};
static void rst(void){memset(feature_report_buf,0,65);feature_report_buf[0]=REPORT_ID;}
static void kpush(const uint8_t *d,uint8_t n){if(kqc>=KQ_SIZE)return;memcpy(kq[kqh].d,d,n);kq[kqh].n=n;kqh=(kqh+1)&KQ_MASK;kqc++;}
static void kreport(void){uint8_t p[9]={1,pio_kmods,0,0,0,0,0,0,0};for(uint8_t i=0;i<pio_kcount&&i<6;i++)p[3+i]=pio_kkeys[i];kpush(p,9);}
static void kpress(uint8_t mods,const uint8_t *keys,uint8_t n){uint8_t p[9]={1,mods,0,0,0,0,0,0,0};for(uint8_t i=0;i<n&&i<6;i++)p[3+i]=keys[i];uint8_t z[9]={1,0,0,0,0,0,0,0,0};kpush(p,9);kpush(z,9);}
static uint8_t hidcode(uint8_t c,uint8_t *sh){
static const char plain[]=" -=[]\\;'`,./";static const uint8_t plain_c[]={0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x33,0x34,0x35,0x36,0x37,0x38};
static const char shifted[]="_+{}|:\"~<>?";static const uint8_t shifted_c[]={0x2D,0x2E,0x2F,0x30,0x31,0x33,0x34,0x35,0x36,0x37,0x38};
static const char digits[]="!@#$%^&*()";*sh=0;
if(c>='a'&&c<='z')return 4+c-'a';if(c>='A'&&c<='Z'){*sh=1;return 4+c-'A';}
if(c>='1'&&c<='9')return 30+c-'1';if(c=='0')return 39;if(c=='\n')return 0x28;if(c=='\b')return 0x2A;if(c=='\t')return 0x2B;
for(uint8_t i=0;i<sizeof(digits)-1;i++)if(c==digits[i]){*sh=1;return 30+i;}
for(uint8_t i=0;i<sizeof(plain)-1;i++)if(c==plain[i])return plain_c[i];
for(uint8_t i=0;i<sizeof(shifted)-1;i++)if(c==shifted[i]){*sh=1;return shifted_c[i];}return 0;
}
static void resolve(uint8_t k,uint8_t *mods,uint8_t *key){
if(k>=224&&k<=231){*mods|=1u<<(k-224);return;}if(k>=136){*key=k-136;return;}if(k>=128){*mods|=1u<<(k-128);return;}
uint8_t sh;uint8_t c=hidcode(k,&sh);if(!c)return;*key=c;if(sh)*mods|=0x02;
}
static void type_text(const char *s){for(;*s;s++){uint8_t sh,c=hidcode((uint8_t)*s,&sh);if(c)kpress(sh?0x02:0,&c,1);}}
static void type_key(uint8_t k){uint8_t mods=0,key=0;resolve(k,&mods,&key);if(mods||key)kpress(mods,&key,key?1:0);}
static void type_combo(const uint8_t *k,uint8_t n){uint8_t mods=0,keys[6],cnt=0;for(uint8_t i=0;i<n;i++){uint8_t m=0,key=0;resolve(k[i],&m,&key);mods|=m;if(key&&cnt<6)keys[cnt++]=key;}if(mods||cnt)kpress(mods,keys,cnt);}
static void type_media(uint8_t m){if(m>12)return;uint8_t p[3]={2,(uint8_t)(media_map[m]&0xFF),(uint8_t)(media_map[m]>>8)};uint8_t z[3]={2,0,0};kpush(p,3);kpush(z,3);}
static void pio_stop(void){pio_active=0;pio_text_active=0;pio_mouse_mask=0;pio_kmods=0;pio_kcount=0;memset(pio_kkeys,0,sizeof(pio_kkeys));kreport();}
static void pio_start(void){
if(g_cfg.pio_mode!=5)return;
if(g_cfg.pio_program[0]!=PIO_MAGIC||g_cfg.pio_program[1]!=PIO_VERSION)return;
uint16_t n=g_cfg.pio_program[2]|((uint16_t)g_cfg.pio_program[3]<<8);
if(n==0||n>444)return;
pio_pos=4;pio_end=4+n;if(g_cfg.pio_program[pio_end-1]!=0)return;
pio_wait=0;pio_text_active=0;pio_active=1;
}
static void pio_trigger_match(uint8_t ev){
uint8_t mode=g_cfg.pio_trigger;
if(mode==3){if(ev==1)pio_start();else if(ev==2)pio_stop();return;}
if((mode==0&&ev==1)||(mode==1&&ev==2)||(mode==2&&(ev==1||ev==2))){
if(g_cfg.pio_mode==5)pio_start();else hid_type_program_me();
}
}
static void pio_key_down(uint8_t k){
if(k>=224&&k<=231){pio_kmods|=(uint8_t)(1u<<(k-224));kreport();return;}
for(uint8_t i=0;i<pio_kcount;i++)if(pio_kkeys[i]==k)return;
if(pio_kcount<6)pio_kkeys[pio_kcount++]=k;kreport();
}
static void pio_key_up(uint8_t k){
if(k>=224&&k<=231){pio_kmods&=(uint8_t)~(1u<<(k-224));kreport();return;}
for(uint8_t i=0;i<pio_kcount;i++)if(pio_kkeys[i]==k){for(uint8_t j=i+1;j<pio_kcount;j++)pio_kkeys[j-1]=pio_kkeys[j];pio_kcount--;break;}kreport();
}
static void pio_step(void){
if(!pio_active)return;
uint32_t now=millis();
if(pio_text_active){
if(now<pio_wait)return;
if(pio_text_pos<pio_text_len){uint8_t sh,c=hidcode((uint8_t)pio_text_ptr[pio_text_pos++],&sh);if(c)kpress(sh?0x02:0,&c,1);pio_wait=now+1;return;}
pio_text_active=0;
}
if(pio_wait&&now<pio_wait)return;
if(pio_pos>=pio_end){pio_stop();return;}
uint8_t op=g_cfg.pio_program[pio_pos++];
switch(op){
case 0:pio_stop();break;
case 1:if(pio_pos+2>pio_end){pio_stop();break;}pio_wait=now+(uint32_t)(g_cfg.pio_program[pio_pos]|((uint16_t)g_cfg.pio_program[pio_pos+1]<<8));pio_pos+=2;break;
case 2:if(pio_pos<pio_end){pio_mouse_mask|=(uint8_t)(1u<<g_cfg.pio_program[pio_pos++]);}break;
case 3:if(pio_pos<pio_end){pio_mouse_mask&=(uint8_t)~(1u<<g_cfg.pio_program[pio_pos++]);}break;
case 4:if(pio_pos<pio_end)pio_key_down(g_cfg.pio_program[pio_pos++]);break;
case 5:if(pio_pos<pio_end)pio_key_up(g_cfg.pio_program[pio_pos++]);break;
case 6:if(pio_pos<pio_end){uint8_t k=g_cfg.pio_program[pio_pos++];uint8_t mods=0,key=0;resolve(k,&mods,&key);if(mods||key)kpress(mods,&key,key?1:0);}break;
case 7:if(pio_pos<pio_end){uint8_t n=g_cfg.pio_program[pio_pos++];if(pio_pos+n>pio_end)n=(uint8_t)(pio_end-pio_pos);pio_text_ptr=(const char*)(g_cfg.pio_program+pio_pos);pio_text_len=n;pio_text_pos=0;pio_pos+=n;pio_text_active=1;pio_wait=0;}break;
case 8:if(pio_pos<pio_end)type_media(g_cfg.pio_program[pio_pos++]);break;
case 9:if(pio_pos<pio_end)pio_wheel+=(int8_t)g_cfg.pio_program[pio_pos++];break;
case 10:if(pio_pos+2<=pio_end){pio_dx+=(int8_t)g_cfg.pio_program[pio_pos++];pio_dy+=(int8_t)g_cfg.pio_program[pio_pos++];}break;
case 11:if(pio_pos<pio_end){uint8_t n=g_cfg.pio_program[pio_pos++];if(pio_pos+n>pio_end)n=(uint8_t)(pio_end-pio_pos);type_combo(g_cfg.pio_program+pio_pos,n);pio_pos+=n;}break;
case 12:pio_mouse_mask=0;pio_kmods=0;pio_kcount=0;memset(pio_kkeys,0,sizeof(pio_kkeys));kreport();break;
default:pio_stop();break;
}
}
extern "C"{
void hid_init(void){webhid_init();rst();pend=0;kqh=kqt=kqc=0;mb_mask=mb_frames=slot_cnt=0;pio_mouse_mask=pio_click_mask=pio_click_frames=0;pio_dx=pio_dy=pio_wheel=0;pio_pending=pio_active=0;last_activity=millis();}
void hid_set_report(uint8_t t,uint8_t id,const uint8_t *d,uint16_t l){(void)t;if(id!=REPORT_ID||!d)return;if(l>64){d++;l--;}if(l>64)l=64;memcpy(req,d,l);reql=l;pend=1;}
uint16_t hid_get_report(uint8_t t,uint8_t id,uint8_t *o,uint16_t m){(void)t;if(id!=REPORT_ID)return 0;if(m>65)m=65;memcpy(o,feature_report_buf,m);return m;}
void hid_task(void){if(!pend)return;uint8_t resp[64]={0};uint16_t rl=0;uint16_t l=reql;if(webhid_handle(REPORT_ID,req,l,resp,&rl)!=0){resp[0]=l>0?req[0]:0;resp[1]=l>1?req[1]:0;resp[2]=0;resp[3]=0;resp[4]=1;rl=5;}if(rl>64)rl=64;rst();memcpy(feature_report_buf+1,resp,rl);__sync_synchronize();pend=0;}
uint8_t hid_slot(void){if(!usb_device_configured())return 0;if(!g_cfg.sof_sync)return usb_ep_ready(USB_EP_MOUSE);if(!usb_device_sof())return 0;uint16_t div=1000/(g_cfg.poll?g_cfg.poll:1000);if(div<1)div=1;if(++slot_cnt<div)return 0;slot_cnt=0;return usb_ep_ready(USB_EP_MOUSE);}
void hid_send(uint8_t b,int8_t w,int16_t dx,int16_t dy){
uint8_t btn=(uint8_t)((b&0x0F)|pio_mouse_mask);
if(mb_frames){mb_frames--;btn|=mb_mask;}
if(pio_click_frames){pio_click_frames--;btn|=pio_click_mask;if(!pio_click_frames)pio_click_mask=0;}
int32_t x=(int32_t)dx+pio_dx,y=(int32_t)dy+pio_dy;pio_dx=0;pio_dy=0;
int16_t ww=(int16_t)w+pio_wheel;pio_wheel=0;
if(x>127)x=127;if(x<-127)x=-127;if(y>127)y=127;if(y<-127)y=-127;if(ww>127)ww=127;if(ww<-127)ww=-127;
uint8_t r[4]={btn,(uint8_t)(int8_t)x,(uint8_t)(int8_t)y,(uint8_t)(int8_t)ww};usb_ep_send(USB_EP_MOUSE,r,4);
if(btn||ww||x||y)last_activity=millis();
}
void hid_service(void){
if(pio_pending){uint8_t ev=pio_pending;pio_pending=0;if(ev&1)pio_trigger_match(1);if(ev&2)pio_trigger_match(2);}
pio_step();
if(!kqc||!usb_ep_ready(USB_EP_KBD))return;
if(usb_ep_send(USB_EP_KBD,kq[kqt].d,kq[kqt].n)){kqt=(kqt+1)&KQ_MASK;kqc--;}
}
void hid_pio_trigger(uint8_t event){if(event==1||event==2)pio_pending|=event;}
void hid_type_program_me(void){
switch(g_cfg.pio_mode){
case 1:mb_mask=(uint8_t)(1u<<(g_cfg.pio_mouse&7));mb_frames=CLICK_FRAMES;break;
case 2:type_key(g_cfg.pio_key);break;
case 3:type_combo(g_cfg.pio_combo,g_cfg.pio_combo_len);break;
case 4:type_media(g_cfg.pio_media);break;
case 5:pio_start();break;
default:type_text(g_cfg.pio_text);
}
}
uint32_t hid_last_activity_ms(void){return last_activity;}
}
