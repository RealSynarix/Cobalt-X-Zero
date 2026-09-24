#include "sensor.h"
#include <Arduino.h>
#include <SPI.h>
#include "srom_04.h"
#define REG_Motion 0x02
#define REG_Config1 0x0F
#define REG_Config2 0x10
#define REG_SROM_Enable 0x13
#define REG_Power_Up_Reset 0x3A
#define REG_Motion_Burst 0x50
#define REG_SROM_Load_Burst 0x62
#define REG_Lift_Config 0x63
static void cs_low(){digitalWrite(PA4,LOW);}static void cs_high(){digitalWrite(PA4,HIGH);}
static uint8_t rreg(uint8_t a){SPI.beginTransaction(SPISettings(2000000,MSBFIRST,SPI_MODE3));cs_low();SPI.transfer(a & 0x7F);delayMicroseconds(35);uint8_t d=SPI.transfer(0);cs_high();SPI.endTransaction();delayMicroseconds(50);return d;}
static void wreg(uint8_t a,uint8_t d){SPI.beginTransaction(SPISettings(2000000,MSBFIRST,SPI_MODE3));cs_low();SPI.transfer(a | 0x80);SPI.transfer(d);delayMicroseconds(20);cs_high();SPI.endTransaction();delayMicroseconds(50);}
void sensor_init(){pinMode(PA2,OUTPUT);digitalWrite(PA2,HIGH);delay(200);pinMode(PA4,OUTPUT);cs_high();delay(100);SPI.setMISO(PA6);SPI.setMOSI(PA7);SPI.setSCLK(PA5);SPI.begin();delay(200);wreg(REG_Power_Up_Reset,0x5A);delay(80);rreg(REG_Motion);delay(10);wreg(REG_Config2,0x00);delay(10);wreg(REG_SROM_Enable,0x1D);delay(10);wreg(REG_SROM_Enable,0x18);delay(10);SPI.beginTransaction(SPISettings(2000000,MSBFIRST,SPI_MODE3));cs_low();SPI.transfer(REG_SROM_Load_Burst | 0x80);delayMicroseconds(15);for(uint16_t i=0;i<SROM_04_LENGTH;i++){SPI.transfer(SROM_04[i]);delayMicroseconds(15);}cs_high();SPI.endTransaction();delay(10);delay(20);wreg(REG_Config1,0x09);delay(10);wreg(REG_Lift_Config,0x02);delay(10);wreg(REG_Config2,0x00);delay(30);rreg(REG_Motion);delay(10);}
uint8_t sensor_raw(int16_t *dx,int16_t *dy,uint8_t *squal,uint8_t *motion, uint8_t *raw_sum, uint8_t *max_raw, uint8_t *min_raw, uint16_t *shutter){
 uint8_t data[12];
 SPI.beginTransaction(SPISettings(2000000,MSBFIRST,SPI_MODE3));
 cs_low();
 SPI.transfer(REG_Motion_Burst);
 delayMicroseconds(35);
 for(uint8_t i=0;i<12;i++){data[i]=SPI.transfer(0);}
 cs_high();
 SPI.endTransaction();
 *motion=data[0];
 *squal=data[6];
 *raw_sum=data[7];
 *max_raw=data[8];
 *min_raw=data[9];
 *shutter = ((uint16_t)data[10]<<8)|data[11];
 if((data[0] & 0x80)==0){*dx=0;*dy=0;return data[0];}
 if(data[0] & 0x08){*dx=0;*dy=0;return data[0];}
 int16_t rx=(int16_t)((data[3]<<8)|data[2]);
 int16_t ry=(int16_t)((data[5]<<8)|data[4]);
 *dx=-rx;
 *dy=-ry;

 // v60 - LESS AGGRESSIVE direct cut, keep normal movement easy
 uint8_t squal_v = data[6];
 uint8_t maxr = data[8];
 uint8_t minr = data[9];
 uint16_t shut = *shutter;
 uint8_t contrast = (maxr>minr)?(maxr-minr):0;
 int16_t vel = abs(*dx)+abs(*dy);

 // Only cut extreme garbage, not normal small moves
 if(squal_v < 20){ *dx=0; *dy=0; return data[0]; } // very low quality
 if(contrast < 12){ *dx=0; *dy=0; return data[0]; }
 if(shut > 30000){ *dx=0; *dy=0; return data[0]; }
 // Tiny move with low quality = likely noise, but allow if quality decent
 if(vel>0 && vel<5 && squal_v < 45){ *dx=0; *dy=0; return data[0]; }
 if(vel>0 && vel<10 && squal_v < 32 && contrast < 25){ *dx=0; *dy=0; return data[0]; }

 return data[0];
}
