#include "sensor_filter.h"
#include <math.h>
#define SF_MAX_PACKETS 8
#define SF_GLITCH 900
#define SF_SQUAL_HYST 2
#define SF_MEDIAN_SLACK 8.0f
#define SF_DIR_A 0.04f
#define SF_SPD_A 0.15f
#define SF_LO_IPS 1.0f
#define SF_HI_IPS 14.0f
#define SF_SNAP_MIN_IPS 1.0f
static float bx,by;
static float spd;
static float dirx,diry;
static uint8_t tracking=1;
void sf_reset(void){bx=by=0.0f;spd=0.0f;dirx=diry=0.0f;tracking=1;}
static inline float clampf(float v,float lo,float hi){return v<lo?lo:(v>hi?hi:v);}
static float median(const int16_t *v,uint8_t k){int16_t s[8];for(uint8_t i=0;i<k;i++){int16_t x=v[i];uint8_t j=i;while(j>0&&s[j-1]>x){s[j]=s[j-1];j--;}s[j]=x;}return (k&1)?(float)s[k>>1]:0.5f*((float)s[(k>>1)-1]+(float)s[k>>1]);}
static float robust_sum(const int16_t *v,uint8_t k){float m=median(v,k);float t=SF_MEDIAN_SLACK+fabsf(m);float sum=0.0f;for(uint8_t i=0;i<k;i++){float x=(float)v[i];sum+=(fabsf(x-m)>t)?m:x;}return sum;}
static void snap(float *fx,float *fy,const sf_params_t *prm,float cpf){float ax=fabsf(dirx),ay=fabsf(diry);if(hypotf(dirx,diry)<SF_SNAP_MIN_IPS*cpf)return;float s=(float)(prm->snap_strength>100?100:prm->snap_strength);float w=(1.5f+0.06f*s)*0.017453293f;float k=0.35f+0.0065f*s;if(ay<=ax){float r=ay/ax;if(r<w){float q=r/w;*fy*=1.0f-k*(1.0f-q*q);}}else{float r=ax/ay;if(r<w){float q=r/w;*fx*=1.0f-k*(1.0f-q*q);}}}
uint8_t sf_frame(const sf_packet_t *p,uint8_t n,const sf_params_t *prm,float *ox,float *oy){
*ox=0.0f;*oy=0.0f;
if(n>SF_MAX_PACKETS)n=SF_MAX_PACKETS;
float cpf=(float)(prm->dpi<100?100:prm->dpi)/1000.0f;
float fx=0.0f,fy=0.0f;
if(n){
int16_t vx[8],vy[8];
uint8_t k=0;
for(uint8_t i=0;i<n;i++){
uint16_t need=prm->squal_min+(tracking?0:SF_SQUAL_HYST);
if(p[i].lifted||p[i].squal<need){tracking=0;continue;}
tracking=1;
int32_t ax=p[i].dx<0?-(int32_t)p[i].dx:p[i].dx;
int32_t ay=p[i].dy<0?-(int32_t)p[i].dy:p[i].dy;
if(ax>SF_GLITCH||ay>SF_GLITCH)continue;
vx[k]=p[i].dx;vy[k]=p[i].dy;k++;
}
if(k==0){tracking=0;return 0;}
float sc=(float)n/(float)k;
fx=robust_sum(vx,k)*sc;
fy=robust_sum(vy,k)*sc;
}
dirx+=(fx-dirx)*SF_DIR_A;
diry+=(fy-diry)*SF_DIR_A;
if(prm->snap_en)snap(&fx,&fy,prm,cpf);
float sp=hypotf(fx,fy);
spd+=(sp-spd)*SF_SPD_A;
float lo=SF_LO_IPS*cpf,hi=SF_HI_IPS*cpf;
float t=clampf((spd-lo)/(hi-lo),0.0f,1.0f);
t=t*t*(3.0f-2.0f*t);
float am=clampf(prm->a_min,0.1f,1.0f);
float a=am+(1.0f-am)*t;
bx+=fx;by+=fy;
*ox=bx*a;*oy=by*a;
bx-=*ox;by-=*oy;
if(fabsf(bx)<0.01f)bx=0.0f;
if(fabsf(by)<0.01f)by=0.0f;
return 1;
}