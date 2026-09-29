#include "device.h"
#include "descriptors.h"
#include "hid.h"
#include "../Config-Module/config.h"
#include <Arduino.h>
#include <stm32g4xx.h>
#include <string.h>
namespace {
constexpr uintptr_t PMA_BASE = USB_BASE + 0x400u;
constexpr uint16_t ISTR_CTR=0x8000, ISTR_RESET=0x0400, ISTR_FLAGS=0x7F00;
constexpr uint16_t CNTR_FRES=0x0001, DADDR_EF=0x0080, BCDR_DPPU=0x8000;
constexpr uint16_t EP_CTR_RX=0x8000, EP_DTOG_RX=0x4000, EP_STAT_RX=0x3000, EP_SETUP=0x0800;
constexpr uint16_t EP_TYPE=0x0600, EP_KIND=0x0100, EP_CTR_TX=0x0080, EP_DTOG_TX=0x0040, EP_STAT_TX=0x0030, EP_ADDR=0x000F;
constexpr uint16_t EP_KEEP=EP_TYPE|EP_KIND|EP_ADDR, EP_CONTROL=0x0200, EP_INTERRUPT=0x0600;
constexpr uint16_t DISABLED=0, STALLED=1, NAK=2, VALID=3;
constexpr uint8_t TX_ADDR=0, TX_COUNT=1, RX_ADDR=2, RX_COUNT=3;
constexpr uint16_t EP0_SIZE=64, EP0_TX_PMA=0x40, EP0_RX_PMA=0x80;
constexpr uint16_t EP_PMA[4]={0,0xC0,0x100,0x140};
constexpr uint16_t RX_COUNT_64=0x8400;
constexpr uint8_t EP_VENDOR=1, EP_MOUSE=2, EP_KBD=3, IF_VENDOR=0, IF_COUNT=3;
constexpr uint8_t HID_OFF[IF_COUNT]={18,43,68};
const uint8_t vendor_desc[] = COBALT_VENDOR_DESC;
const uint8_t mouse_desc[] = COBALT_MOUSE_DESC;
const uint8_t kbd_desc[] = COBALT_KBD_DESC;
const uint8_t *const rep_desc[IF_COUNT]={vendor_desc,mouse_desc,kbd_desc};
const uint16_t rep_len[IF_COUNT]={sizeof(vendor_desc),sizeof(mouse_desc),sizeof(kbd_desc)};
const uint8_t device_desc[] = {18,1,0,2,0,0,0,EP0_SIZE, USB_VID&0xFF, USB_VID>>8, USB_PID&0xFF, USB_PID>>8, 1,1, 1,2,3,1};
const uint8_t config_desc[] = {
9,2,84,0, 3,1,0, 0x80,50,
9,4, 0,0,1, 3,0,0,0,
9,0x21, 0x11,1, 0,1, 0x22, sizeof(vendor_desc)&0xFF, sizeof(vendor_desc)>>8,
7,5, 0x80|EP_VENDOR, 3, 64,0, 10,
9,4, 1,0,1, 3,0,0,0,
9,0x21, 0x11,1, 0,1, 0x22, sizeof(mouse_desc)&0xFF, sizeof(mouse_desc)>>8,
7,5, 0x80|EP_MOUSE, 3, 8,0, 1,
9,4, 2,0,1, 3,0,0,0,
9,0x21, 0x11,1, 0,1, 0x22, sizeof(kbd_desc)&0xFF, sizeof(kbd_desc)>>8,
7,5, 0x80|EP_KBD, 3, 16,0, 1,
};
struct StrSrc{ const char* v; uint8_t m; const char* fb; };
const StrSrc strs[] = {
{g_cfg.manufacturer, sizeof(g_cfg.manufacturer), USB_MANUFACTURER_STRING},
{g_cfg.device_name, sizeof(g_cfg.device_name), USB_PRODUCT_STRING},
{g_cfg.serial, sizeof(g_cfg.serial), "0"},
};
struct Req{ uint8_t t,r; uint16_t va,in,le; }; enum St:uint8_t{IDLE,DI,DO,SI,SO}; Req cur; St st=IDLE;
const uint8_t *txp; uint16_t txl; bool tz; uint8_t cb[72]; uint16_t og; uint8_t na; bool ap; uint8_t proto=1; volatile uint8_t cfgd; volatile uint16_t lfn=0xFFFF;
inline volatile uint16_t& epr(uint8_t n){ return *reinterpret_cast<volatile uint16_t*>(USB_BASE+4u*n); }
inline volatile uint16_t& pma(uint16_t o){ return *reinterpret_cast<volatile uint16_t*>(PMA_BASE+o); }
inline volatile uint16_t& bt(uint8_t n,uint8_t f){ return pma(n*8u+f*2u); }
void pw(uint16_t o,const uint8_t* s,uint16_t l){ for(uint16_t i=0;i<l;i+=2){ uint16_t h=i+1<l?s[i+1]:0; pma(o+i)=s[i]|(h<<8); } }
void pr(uint16_t o,uint8_t* d,uint16_t l){ for(uint16_t i=0;i<l;i+=2){ uint16_t w=pma(o+i); d[i]=w&0xFF; if(i+1<l) d[i+1]=w>>8; } }
void srx(uint8_t n,uint16_t s){ uint16_t r=epr(n); epr(n)=(r&EP_KEEP)|EP_CTR_RX|EP_CTR_TX|((r ^ (s<<12))&EP_STAT_RX); }
void stx(uint8_t n,uint16_t s){ uint16_t r=epr(n); epr(n)=(r&EP_KEEP)|EP_CTR_RX|EP_CTR_TX|((r ^ (s<<4))&EP_STAT_TX); }
void crx(uint8_t n){ epr(n)=(epr(n)&EP_KEEP)|EP_CTR_TX; }
void ctx(uint8_t n){ epr(n)=(epr(n)&EP_KEEP)|EP_CTR_RX; }
void oep(uint8_t n,uint16_t ty,uint16_t rx,uint16_t tx){ uint16_t r=epr(n); epr(n)=ty|n|EP_CTR_RX|EP_CTR_TX|(r&(EP_DTOG_RX|EP_DTOG_TX))|((r ^ (rx<<12))&EP_STAT_RX)|((r ^ (tx<<4))&EP_STAT_TX); }
void oiep(uint8_t n,uint16_t off){ bt(n,TX_ADDR)=off; bt(n,TX_COUNT)=0; oep(n,EP_INTERRUPT,DISABLED,NAK); }
void brst(void){ USB->DADDR=DADDR_EF; bt(0,TX_ADDR)=EP0_TX_PMA; bt(0,TX_COUNT)=0; bt(0,RX_ADDR)=EP0_RX_PMA; bt(0,RX_COUNT)=RX_COUNT_64; oep(0,EP_CONTROL,VALID,NAK); for(uint8_t n=EP_VENDOR;n<=EP_KBD;n++) oep(n,EP_INTERRUPT,DISABLED,DISABLED); st=IDLE; ap=false; cfgd=0; proto=1; }
void stall(void){ stx(0,STALLED); srx(0,STALLED); st=IDLE; }
void schunk(void){ uint16_t n=txl<EP0_SIZE?txl:EP0_SIZE; pw(EP0_TX_PMA,txp,n); bt(0,TX_COUNT)=n; txp+=n; txl-=n; stx(0,VALID); }
void repl(const uint8_t* d,uint16_t l){ if(l>cur.le) l=cur.le; txp=d; txl=l; tz=l && l<cur.le && l%EP0_SIZE==0; st=DI; schunk(); srx(0,VALID); }
void repb(uint8_t v){ cb[0]=v; repl(cb,1); }
void ack(void){ st=SI; bt(0,TX_COUNT)=0; stx(0,VALID); srx(0,VALID); }
void eout(void){ st=DO; og=0; srx(0,VALID); }
uint16_t bstr(uint8_t idx,uint8_t* out){
if(idx==0){ out[0]=4; out[1]=3; out[2]=9; out[3]=4; return 4; }
if(idx>3) return 0;
const StrSrc& s=strs[idx-1]; const char* t=s.v[0]?s.v:s.fb; uint8_t mx=s.v[0]?s.m:32; size_t n=strnlen(t,mx);
out[0]=2+2*n; out[1]=3; for(size_t i=0;i<n;i++){ out[2+2*i]=t[i]; out[3+2*i]=0; } return out[0];
}
void gdesc(void){
uint8_t k=cur.va>>8; uint8_t ix=cur.va&0xFF;
if((cur.t&0x1F)==1){ if(cur.in>=IF_COUNT){ stall(); return; } if(k==0x22) repl(rep_desc[cur.in],rep_len[cur.in]); else if(k==0x21) repl(config_desc+HID_OFF[cur.in],9); else stall(); return; }
switch(k){ case 1: repl(device_desc,sizeof(device_desc)); break; case 2: repl(config_desc,sizeof(config_desc)); break; case 3:{ uint16_t n=bstr(ix,cb); n?repl(cb,n):stall(); break; } default: stall(); }
}
void scfg(void){ if(cur.va>1){ stall(); return; } cfgd=cur.va; for(uint8_t n=EP_VENDOR;n<=EP_KBD;n++){ if(cfgd) oiep(n,EP_PMA[n]); else oep(n,EP_INTERRUPT,DISABLED,DISABLED); } ack(); }
void sreq(void){ switch(cur.r){ case 0: cb[0]=0; cb[1]=0; repl(cb,2); break; case 1: case 3: case 0x0B: ack(); break; case 5: na=cur.va&0x7F; ap=true; ack(); break; case 6: gdesc(); break; case 8: repb(cfgd); break; case 9: scfg(); break; case 0x0A: repb(0); break; default: stall(); } }
void hreq(void){
uint8_t k=cur.va>>8; uint8_t id=cur.va&0xFF;
switch(cur.r){
case 1: if(cur.in==IF_VENDOR && k==3){ uint16_t n=hid_get_report(k,id,cb,cur.le<sizeof(cb)?cur.le:sizeof(cb)); n?repl(cb,n):stall(); } else stall(); break;
case 9: if(cur.in<IF_COUNT && (k==2||k==3) && cur.le && cur.le<=sizeof(cb)) eout(); else stall(); break;
case 2: repb(0); break; case 3: repb(proto); break; case 0x0A: ack(); break; case 0x0B: proto=cur.va&0xFF; ack(); break; default: stall();
}
}
void e0s(void){ uint8_t raw[8]; pr(EP0_RX_PMA,raw,8); crx(0); stx(0,NAK); cur.t=raw[0]; cur.r=raw[1]; cur.va=raw[2]|(raw[3]<<8); cur.in=raw[4]|(raw[5]<<8); cur.le=raw[6]|(raw[7]<<8); switch(cur.t&0x60){ case 0: sreq(); break; case 0x20: hreq(); break; default: stall(); } }
void e0o(void){ if(st!=DO){ crx(0); st=IDLE; srx(0,VALID); return; } uint16_t n=bt(0,RX_COUNT)&0x3FF; uint16_t room=sizeof(cb)-og; uint16_t take=n<room?n:room; pr(EP0_RX_PMA,cb+og,take); og+=take; crx(0); if(og>=cur.le || n<EP0_SIZE){ if(cur.in==IF_VENDOR) hid_set_report(cur.va>>8, cur.va&0xFF, cb, og); ack(); } else srx(0,VALID); }
void e0i(void){ ctx(0); if(st==SI){ if(ap){ USB->DADDR=DADDR_EF|na; ap=false; } st=IDLE; } else if(st==DI){ if(txl) schunk(); else if(tz){ tz=false; schunk(); } else st=SO; } }
}
extern "C" {
void usb_device_init(void){
NVIC_DisableIRQ(USB_HP_IRQn); NVIC_DisableIRQ(USB_LP_IRQn); NVIC_DisableIRQ(USBWakeUp_IRQn);
RCC->APB1ENR1|=RCC_APB1ENR1_USBEN; RCC->APB1RSTR1|=RCC_APB1RSTR1_USBRST; RCC->APB1RSTR1&=~RCC_APB1RSTR1_USBRST; __DSB();
USB->CNTR=CNTR_FRES; for(uint32_t i=0;i<256;i++) __NOP(); USB->CNTR=0; USB->ISTR=0; USB->BTABLE=0; USB->BCDR=0;
brst(); lfn=0xFFFF; delay(20); USB->BCDR=BCDR_DPPU;
}
void usb_device_poll(void){
uint16_t f=USB->ISTR; if(f&ISTR_FLAGS) USB->ISTR=~(f&ISTR_FLAGS); if(f&ISTR_RESET) brst();
for(uint8_t g=0;g<8;g++){ uint16_t s=USB->ISTR; if(!(s&ISTR_CTR)) break; uint8_t n=s&EP_ADDR; uint16_t r=epr(n);
if(n==0){ if(r&EP_CTR_TX) e0i(); if(r&EP_CTR_RX) (r&EP_SETUP)?e0s():e0o(); } else { if(r&EP_CTR_TX) ctx(n); if(r&EP_CTR_RX) crx(n); }
}
}
uint8_t usb_device_sof(void){ uint16_t c=USB->FNR & USB_FNR_FN; if(c==lfn) return 0; lfn=c; return 1; }
uint8_t usb_device_configured(void){ return cfgd; }
uint8_t usb_ep_ready(uint8_t ep){ return (epr(ep)&EP_STAT_TX)!= (VALID<<4); }
uint8_t usb_ep_send(uint8_t ep,const uint8_t *data,uint8_t len){
if(!usb_ep_ready(ep)) return 0;
if(len>64) len=64;
pw(EP_PMA[ep],data,len);
bt(ep,TX_COUNT)=len;
stx(ep,VALID);
return 1;
}
}
