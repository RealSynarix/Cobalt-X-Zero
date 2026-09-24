#include "src/MCU-Module/clock.h"
#include "src/USB-Module/device.h"
#include "src/USB-Module/hid.h"
#include "src/Sensor-Module/sensor.h"
#include <Arduino.h>

// v60 - BALANCED - normal movement easy, automove mostly killed but still present -> more rigorous but less hard
// Fix: less aggressive SQUAL cut at sensor, more rigorous idle detection based on time+zero

#define PACK_N 8
#define SNAP_N 3
#define HIST_8K 32
#define GEST_HIST 8

static int16_t pack_x[PACK_N]={0};
static int16_t pack_y[PACK_N]={0};
static uint8_t pack_squal[PACK_N]={0};
static uint8_t pack_contrast[PACK_N]={0};
static uint16_t pack_shutter[PACK_N]={0};
static uint8_t pack_c=0;

static int16_t raw8k_x[HIST_8K]={0};
static int16_t raw8k_y[HIST_8K]={0};
static uint8_t raw8k_idx=0;

static int16_t snap_x[SNAP_N]={0};
static int16_t snap_y[SNAP_N]={0};
static uint8_t snap_idx=0;

static int16_t gest_hist_x[GEST_HIST]={0};
static int16_t gest_hist_y[GEST_HIST]={0};
static uint8_t gest_idx=0;

static int32_t rem_x=0,rem_y=0;
static bool is_idle=true;
static uint8_t zero_ms=0,lift_low=0,lift_high=0;
static bool lifted=false;
static uint32_t t_sens=0,t_usb=0,t_ms=0;

static int16_t last_sum_x=0,last_sum_y=0;
static uint16_t same_dir_cnt=0,zero_accel_cnt=0,no_zero_cnt=0;
static uint32_t drift_start_ms=0;
static uint32_t last_zero_ms=0;
static uint16_t low_vel_cnt=0;

static int16_t precreated_x=0, precreated_y=0;
static bool packet_ready=false;

static void enter_idle(){
 is_idle=true; pack_c=0; zero_ms=0; rem_x=0; rem_y=0;
 precreated_x=0; precreated_y=0; packet_ready=false;
 same_dir_cnt=0; zero_accel_cnt=0; no_zero_cnt=0; drift_start_ms=0; low_vel_cnt=0;
 for(uint8_t i=0;i<PACK_N;i++){ pack_x[i]=0; pack_y[i]=0; pack_squal[i]=0; pack_contrast[i]=0; pack_shutter[i]=0; }
}

static void push_8k(int16_t dx,int16_t dy){ raw8k_x[raw8k_idx]=dx; raw8k_y[raw8k_idx]=dy; raw8k_idx=(raw8k_idx+1)%HIST_8K; }
static void push_snap(int16_t dx,int16_t dy){ snap_x[snap_idx]=dx; snap_y[snap_idx]=dy; snap_idx=(snap_idx+1)%SNAP_N; }
static void push_gest(int16_t sx,int16_t sy){ gest_hist_x[gest_idx]=sx; gest_hist_y[gest_idx]=sy; gest_idx=(gest_idx+1)%GEST_HIST; }

static void snapshot_cutout(int16_t *dx,int16_t *dy){
 int16_t sx0=snap_x[(snap_idx-1+SNAP_N)%SNAP_N];
 int16_t sy0=snap_y[(snap_idx-1+SNAP_N)%SNAP_N];
 int16_t sx1=snap_x[(snap_idx-2+SNAP_N)%SNAP_N];
 int16_t sy1=snap_y[(snap_idx-2+SNAP_N)%SNAP_N];
 int16_t sx2=snap_x[(snap_idx-3+SNAP_N)%SNAP_N];
 int16_t sy2=snap_y[(snap_idx-3+SNAP_N)%SNAP_N];
 static uint8_t filled=0;
 if(filled<3){ filled++; return; }
 // Only cut very small isolated outliers, not normal moves
 if(abs(sx0)<=10){
  int16_t xs[3]={sx0,sx1,sx2}; uint8_t nz=0; for(uint8_t i=0;i<3;i++) if(abs(xs[i])<=2) nz++;
  if(nz>=2 && abs(sx0)>=4){ *dx=0; }
 }
 if(abs(sy0)<=10){
  int16_t ys[3]={sy0,sy1,sy2}; uint8_t nz=0; for(uint8_t i=0;i<3;i++) if(abs(ys[i])<=2) nz++;
  if(nz>=2 && abs(sy0)>=4){ *dy=0; }
 }
}

static void process_packets(){
 if(pack_c==0) return;
 int16_t sum_x=0,sum_y=0,abs_sum_x=0,abs_sum_y=0;
 uint16_t avg_squal=0,avg_contrast=0; uint32_t avg_shutter=0;
 for(uint8_t i=0;i<pack_c;i++){ sum_x+=pack_x[i]; sum_y+=pack_y[i]; abs_sum_x+=abs(pack_x[i]); abs_sum_y+=abs(pack_y[i]); avg_squal+=pack_squal[i]; avg_contrast+=pack_contrast[i]; avg_shutter+=pack_shutter[i]; }
 avg_squal/=pack_c; avg_contrast/=pack_c; avg_shutter/=pack_c;

 int16_t cur_vel=abs(sum_x)+abs(sum_y);
 int16_t last_vel=abs(last_sum_x)+abs(last_sum_y);
 int16_t accel=cur_vel-last_vel;
 last_sum_x=sum_x; last_sum_y=sum_y;

 bool same_dir=false;
 if(sum_x!=0 && last_sum_x!=0 && ((sum_x>0)==(last_sum_x>0))) same_dir=true;
 if(sum_y!=0 && last_sum_y!=0 && ((sum_y>0)==(last_sum_y>0))) same_dir=true;
 if(same_dir){ same_dir_cnt++; if(abs(accel)<=2) zero_accel_cnt++; else zero_accel_cnt=0; if(drift_start_ms==0) drift_start_ms=t_ms; no_zero_cnt++; }
 else { same_dir_cnt=0; zero_accel_cnt=0; drift_start_ms=0; no_zero_cnt=0; }
 if(cur_vel<=20) low_vel_cnt++; else low_vel_cnt=0;

 // RIGOROUS but not hard - only kill true idle, allow slow intentional
 if(sum_x==0 && sum_y==0){
  zero_ms++; last_zero_ms=t_ms; no_zero_cnt=0; same_dir_cnt=0; drift_start_ms=0; low_vel_cnt=0;
  if(zero_ms>=8 && !is_idle){ enter_idle(); pack_c=0; return; }
 } else {
  is_idle=false; zero_ms=0;
  uint32_t drift_dur=(drift_start_ms==0)?0:t_ms-drift_start_ms;

  // More rigorous: require longer duration + lower SQUAL to kill, so normal slow moves survive
  if(avg_squal < 30 && cur_vel<=40){ enter_idle(); pack_c=0; return; } // very low quality
  if(avg_contrast < 15 && cur_vel<=40){ enter_idle(); pack_c=0; return; }
  if(avg_shutter > 28000 && cur_vel<=50){ enter_idle(); pack_c=0; return; }

  // Continuous tiny drift - more rigorous: need 400ms+ no zero, same dir, low vel
  if(drift_dur>400 && cur_vel>=2 && cur_vel<=45 && no_zero_cnt>=300 && zero_accel_cnt>=10){ enter_idle(); pack_c=0; return; }
  // Very long tiny drift
  if(drift_dur>700 && cur_vel>=2 && cur_vel<=35 && no_zero_cnt>=500){ enter_idle(); pack_c=0; return; }
  // Low vel for 600ms
  if(low_vel_cnt>=600 && cur_vel>=2 && cur_vel<=22){ enter_idle(); pack_c=0; return; }
  // Ultra consistent tiny drift 300ms
  if(drift_dur>300 && cur_vel>=2 && cur_vel<=18 && zero_accel_cnt>=100){ enter_idle(); pack_c=0; return; }

  // Additional: if SQUAL medium-low and no zero for long time, kill
  if(avg_squal < 38 && drift_dur>350 && cur_vel<=30 && no_zero_cnt>=280){ enter_idle(); pack_c=0; return; }
 }

 // 1000 DPI - make easy: 95% scale (was 85% too hard)
 int16_t scaled_x=sum_x*95/100, scaled_y=sum_y*95/100;
 sum_x=scaled_x; sum_y=scaled_y;
 cur_vel=abs(sum_x)+abs(sum_y);

 // Gesture snap - lighter snap so movement not hard
 bool isXOnly=false,isYOnly=false,isDiag=false,isCircle=false;
 if(abs(sum_x)>6 && abs(sum_y) < abs(sum_x)*20/100){
  uint8_t yc=0; for(uint8_t i=1;i<pack_c;i++) if(pack_y[i]!=0 && pack_y[i-1]!=0 && ((pack_y[i]>0)!=(pack_y[i-1]>0))) yc++;
  if(abs_sum_y < 35 || yc>=2) isXOnly=true;
 }
 if(abs(sum_y)>6 && abs(sum_x) < abs(sum_y)*20/100){
  uint8_t xc=0; for(uint8_t i=1;i<pack_c;i++) if(pack_x[i]!=0 && pack_x[i-1]!=0 && ((pack_x[i]>0)!=(pack_x[i-1]>0))) xc++;
  if(abs_sum_x < 35 || xc>=2) isYOnly=true;
 }
 if(!isXOnly && !isYOnly && abs(sum_x)>8 && abs(sum_y)>8){
  int16_t minA=min(abs(sum_x),abs(sum_y)), maxA=max(abs(sum_x),abs(sum_y));
  if(minA*100/maxA>60){
   uint8_t xc=0,yc=0;
   for(uint8_t i=1;i<pack_c;i++){
    if(pack_x[i]!=0 && pack_x[i-1]!=0 && ((pack_x[i]>0)==(pack_x[i-1]>0))) xc++;
    if(pack_y[i]!=0 && pack_y[i-1]!=0 && ((pack_y[i]>0)==(pack_y[i-1]>0))) yc++;
   }
   if(xc>=3 && yc>=3) isDiag=true;
  }
 }
 if(!isXOnly && !isYOnly && !isDiag && cur_vel>15){
  float la[5]={0}; uint8_t l=0;
  for(int8_t i=GEST_HIST-1;i>=0 && l<5;i--){
   int idx=(gest_idx-1-i+GEST_HIST)%GEST_HIST;
   if(gest_hist_x[idx]==0 && gest_hist_y[idx]==0) continue;
   la[l]=atan2f((float)gest_hist_y[idx], (float)gest_hist_x[idx]); l++;
  }
  if(l>=4){
   float d1=la[0]-la[1], d2=la[1]-la[2], d3=la[2]-la[3];
   auto norm=[](float a){ while(a>3.14159f) a-=6.28318f; while(a<-3.14159f) a+=6.28318f; return a; };
   d1=norm(d1); d2=norm(d2); d3=norm(d3);
   if((d1>0.04f && d2>0.04f && d3>0.04f) || (d1<-0.04f && d2<-0.04f && d3<-0.04f)){
    float av=(fabs(d1)+fabs(d2)+fabs(d3))/3.0f;
    if(av<0.6f){
     int16_t vels[5]={0};
     for(uint8_t j=0;j<l;j++){ int idx=(gest_idx-1-j+GEST_HIST)%GEST_HIST; vels[j]=abs(gest_hist_x[idx])+abs(gest_hist_y[idx]); }
     int16_t vavg=0; for(uint8_t j=0;j<l;j++) vavg+=vels[j]; vavg/=l;
     int16_t vstd=0; for(uint8_t j=0;j<l;j++) vstd+=abs(vels[j]-vavg); vstd/=l;
     if(vavg>10 && vstd < vavg*35/100) isCircle=true;
    }
   }
  }
 }

 int16_t final_x=sum_x, final_y=sum_y;
 if(isXOnly){ final_y=final_y*15/100; if(abs(final_y)<2) final_y=0; } // was 5% -> 15% easier
 else if(isYOnly){ final_x=final_x*15/100; if(abs(final_x)<2) final_x=0; }
 else if(isDiag){
  int16_t avg=(abs(final_x)+abs(final_y))/2;
  int16_t sx=(final_x>0)?avg:-avg, sy=(final_y>0)?avg:-avg;
  final_x=(sx*25 + final_x*75)/100; final_y=(sy*25 + final_y*75)/100; // was 35% -> 25% lighter
 }
 else if(isCircle){
  if(gest_idx>0){ int p=(gest_idx-1+GEST_HIST)%GEST_HIST; final_x=(final_x*85 + gest_hist_x[p]*15)/100; final_y=(final_y*85 + gest_hist_y[p]*15)/100; } // was 80/20 -> 85/15 easier
 } else {
  uint8_t xc=0,yc=0;
  for(uint8_t i=1;i<pack_c;i++){
   if(pack_x[i]!=0 && pack_x[i-1]!=0 && ((pack_x[i]>0)!=(pack_x[i-1]>0))) xc++;
   if(pack_y[i]!=0 && pack_y[i-1]!=0 && ((pack_y[i]>0)!=(pack_y[i-1]>0))) yc++;
  }
  if(xc>=3 && abs(final_x)<abs(final_y)) final_x=final_x*50/100; // was 30% -> 50% easier
  if(yc>=3 && abs(final_y)<abs(final_x)) final_y=final_y*50/100;
 }

 if(abs(final_x)<=14 && abs(final_y)<=14){
  rem_x+=final_x*256; rem_y+=final_y*256;
  int32_t ox=0,oy=0;
  while(rem_x>=256){ ox++; rem_x-=256; } while(rem_x<=-256){ ox--; rem_x+=256; }
  while(rem_y>=256){ oy++; rem_y-=256; } while(rem_y<=-256){ oy--; rem_y+=256; }
  precreated_x=ox; precreated_y=oy; packet_ready=true;
 } else {
  int32_t hsx=0,hsy=0; for(uint8_t i=0;i<HIST_8K;i++){ hsx+=raw8k_x[i]; hsy+=raw8k_y[i]; }
  int16_t hax=hsx/HIST_8K, hay=hsy/HIST_8K;
  precreated_x=(final_x*92 + hax*8)/100; precreated_y=(final_y*92 + hay*8)/100; // was 90/10 -> 92/8 more responsive
  packet_ready=true;
 }
 push_gest(final_x, final_y);
 pack_c=0;
 for(uint8_t i=0;i<PACK_N;i++){ pack_x[i]=0; pack_y[i]=0; pack_squal[i]=0; pack_contrast[i]=0; pack_shutter[i]=0; }
}

void setup(void){
 clock_init(); hid_init(); usb_device_init(); delay(300); sensor_init(); delay(100);
 t_sens=micros(); t_usb=micros(); t_ms=millis(); last_zero_ms=millis();
}
void loop(void){
 uint32_t now=micros();
 if((now - t_sens) >= 125){
  t_sens+=125;
  int16_t dx,dy; uint8_t sq,mot,raw_sum,max_raw,min_raw; uint16_t shutter;
  sensor_raw(&dx,&dy,&sq,&mot,&raw_sum,&max_raw,&min_raw,&shutter);
  push_snap(dx,dy);
  int16_t cdx=dx, cdy=dy;
  snapshot_cutout(&cdx,&cdy);
  uint8_t contrast = (max_raw>min_raw)?(max_raw-min_raw):0;

  // lift less aggressive so normal moves easy
  if(sq < 25) lift_low++; else lift_low=0; // was 35 for 8 -> 25 for more tolerant
  if(sq > 50) lift_high++; else lift_high=0;
  if(!lifted && lift_low>=12){ lifted=true; enter_idle(); }
  else if(lifted && lift_high>=6 && (mot & 0x80)){ lifted=false; lift_low=0; lift_high=0; enter_idle(); }
  else if(!lifted){
   if(contrast < 12 || shutter > 30000 || sq < 20){
    // extreme garbage only
   } else {
    push_8k(cdx,cdy);
    if(pack_c<PACK_N){ pack_x[pack_c]=cdx; pack_y[pack_c]=cdy; pack_squal[pack_c]=sq; pack_contrast[pack_c]=contrast; pack_shutter[pack_c]=shutter; pack_c++; if(pack_c==PACK_N) process_packets(); }
   }
  }
 }
 now=micros();
 if((now - t_usb) >= 1000){
  t_usb+=1000; t_ms=millis();
  if(!packet_ready && pack_c>0) process_packets();
  if(packet_ready){
   int32_t fx=precreated_x, fy=precreated_y;
   precreated_x=0; precreated_y=0; packet_ready=false;
   if(fx>600) fx=600; if(fx<-600) fx=-600; if(fy>600) fy=600; if(fy<-600) fy=-600;
   if(fx!=0 || fy!=0) hid_send((int16_t)fx,(int16_t)fy);
  }
 }
}
