#include "hid.h"
#include <Arduino.h>
#define private public
#include <Mouse.h>
#undef private
void hid_init(void){ Mouse.begin(); }
void hid_send(uint8_t b,int8_t w,int16_t dx,int16_t dy){
  Mouse._buttons = b & 0x07;
  int16_t cx = dx; if(cx>127) cx=127; if(cx<-127) cx=-127;
  int16_t cy = dy; if(cy>127) cy=127; if(cy<-127) cy=-127;
  Mouse.move(cx,cy,w);
}
