#include "hid.h"
#include "device.h"
#include "../Config-Module/config.h"
#include "../Config-Module/webhid.h"
#include <string.h>
#define REPORT_ID 2
#define KQ_SIZE 64
#define KQ_MASK 63
#define CLICK_FRAMES 20
uint8_t feature_report_buf[65];
static uint8_t req[64];
static volatile uint16_t reql=0;
static volatile uint8_t pend=0;
typedef struct{uint8_t n;uint8_t d[9];}krep_t;
static krep_t kq[KQ_SIZE];
static uint8_t kqh=0,kqt=0,kqc=0;
static uint8_t mb_mask=0,mb_frames=0,slot_cnt=0;
static const uint16_t media_map[13]={0x00CD,0x00B5,0x00B6,0x00B7,0x00E2,0x00E9,0x00EA,0x0192,0x0223,0x018A,0x0183,0x006F,0x0070};
static void rst(void){memset(feature_report_buf,0,65);feature_report_buf[0]=REPORT_ID;}
static void kpush(const uint8_t *d,uint8_t n){
if(kqc>=KQ_SIZE)return;
memcpy(kq[kqh].d,d,n);
kq[kqh].n=n;
kqh=(kqh+1)&KQ_MASK;
kqc++;
}
static void kpress(uint8_t mods,const uint8_t *keys,uint8_t n){
uint8_t p[9]={1,mods,0,0,0,0,0,0,0};
for(uint8_t i=0;i<n&&i<6;i++)p[3+i]=keys[i];
uint8_t z[9]={1,0,0,0,0,0,0,0,0};
kpush(p,9);
kpush(z,9);
}
static uint8_t hidcode(uint8_t c,uint8_t *sh){
static const char plain[]=" -=[]\\;'`,./";
static const uint8_t plain_c[]={0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x33,0x34,0x35,0x36,0x37,0x38};
static const char shifted[]="_+{}|:\"~<>?";
static const uint8_t shifted_c[]={0x2D,0x2E,0x2F,0x30,0x31,0x33,0x34,0x35,0x36,0x37,0x38};
static const char digits[]="!@#$%^&*()";
*sh=0;
if(c>='a'&&c<='z')return 4+c-'a';
if(c>='A'&&c<='Z'){*sh=1;return 4+c-'A';}
if(c>='1'&&c<='9')return 30+c-'1';
if(c=='0')return 39;
if(c=='\n')return 0x28;
if(c=='\b')return 0x2A;
if(c=='\t')return 0x2B;
for(uint8_t i=0;i<sizeof(digits)-1;i++)if(c==digits[i]){*sh=1;return 30+i;}
for(uint8_t i=0;i<sizeof(plain)-1;i++)if(c==plain[i])return plain_c[i];
for(uint8_t i=0;i<sizeof(shifted)-1;i++)if(c==shifted[i]){*sh=1;return shifted_c[i];}
return 0;
}
static void resolve(uint8_t k,uint8_t *mods,uint8_t *key){
if(k>=136){*key=k-136;return;}
if(k>=128){*mods|=1u<<(k-128);return;}
uint8_t sh;
uint8_t c=hidcode(k,&sh);
if(!c)return;
*key=c;
if(sh)*mods|=0x02;
}
static void type_text(const char *s){
for(;*s;s++){
uint8_t sh;
uint8_t c=hidcode((uint8_t)*s,&sh);
if(c)kpress(sh?0x02:0,&c,1);
}
}
static void type_key(uint8_t k){
uint8_t mods=0,key=0;
resolve(k,&mods,&key);
if(mods||key)kpress(mods,&key,key?1:0);
}
static void type_combo(const uint8_t *k,uint8_t n){
uint8_t mods=0,keys[6],cnt=0;
for(uint8_t i=0;i<n;i++){
uint8_t m=0,key=0;
resolve(k[i],&m,&key);
mods|=m;
if(key&&cnt<6)keys[cnt++]=key;
}
if(mods||cnt)kpress(mods,keys,cnt);
}
static void type_media(uint8_t m){
if(m>12)return;
uint8_t p[3]={2,(uint8_t)(media_map[m]&0xFF),(uint8_t)(media_map[m]>>8)};
uint8_t z[3]={2,0,0};
kpush(p,3);
kpush(z,3);
}
extern "C"{
void hid_init(void){webhid_init();rst();pend=0;kqh=kqt=kqc=0;mb_mask=mb_frames=slot_cnt=0;}
void hid_set_report(uint8_t t,uint8_t id,const uint8_t *d,uint16_t l){
(void)t;if(id!=REPORT_ID||!d)return;
if(l>64){d++;l--;}
if(l>64)l=64;
memcpy(req,d,l);reql=l;pend=1;
}
uint16_t hid_get_report(uint8_t t,uint8_t id,uint8_t *o,uint16_t m){
(void)t;if(id!=REPORT_ID)return 0;
if(m>65)m=65;
memcpy(o,feature_report_buf,m);
return m;
}
void hid_task(void){
if(!pend)return;
uint8_t resp[64]={0};uint16_t rl=0;uint16_t l=reql;
if(webhid_handle(REPORT_ID,req,l,resp,&rl)!=0){resp[0]=l>0?req[0]:0;resp[1]=l>1?req[1]:0;resp[2]=0;resp[3]=0;resp[4]=1;rl=5;}
if(rl>64)rl=64;
rst();
memcpy(feature_report_buf+1,resp,rl);
__sync_synchronize();
pend=0;
}
uint8_t hid_slot(void){
if(!usb_device_configured())return 0;
if(!g_cfg.mod_sof)return usb_ep_ready(USB_EP_MOUSE);
if(!usb_device_sof())return 0;
uint16_t div=g_cfg.poll>=1000?1:1000/(g_cfg.poll?g_cfg.poll:1000);
if(++slot_cnt<div)return 0;
slot_cnt=0;
return usb_ep_ready(USB_EP_MOUSE);
}
void hid_send(uint8_t b,int8_t w,int16_t dx,int16_t dy){
uint8_t btn=b&0x0F;
if(mb_frames){mb_frames--;btn|=mb_mask;}
if(dx>127)dx=127;
if(dx<-127)dx=-127;
if(dy>127)dy=127;
if(dy<-127)dy=-127;
uint8_t r[4]={btn,(uint8_t)(int8_t)dx,(uint8_t)(int8_t)dy,(uint8_t)w};
usb_ep_send(USB_EP_MOUSE,r,4);
}
void hid_service(void){
if(!kqc||!usb_ep_ready(USB_EP_KBD))return;
if(usb_ep_send(USB_EP_KBD,kq[kqt].d,kq[kqt].n)){kqt=(kqt+1)&KQ_MASK;kqc--;}
}
void hid_type_program_me(void){
switch(g_cfg.pb7_mode){
case 1:mb_mask=(uint8_t)(1u<<(g_cfg.pb7_mouse&7));mb_frames=CLICK_FRAMES;break;
case 2:type_key(g_cfg.pb7_key);break;
case 3:type_combo(g_cfg.pb7_combo,g_cfg.pb7_combo_len);break;
case 4:type_media(g_cfg.pb7_media);break;
default:type_text("Program Me!");
}
}
}
