#include "hid.h"
#include <Arduino.h>
#define private public
#include <Mouse.h>
#include <Keyboard.h>
#undef private
void hid_init(void){Mouse.begin();Keyboard.begin();}
void hid_send(int16_t dx,int16_t dy){Mouse.move(dx,dy,0);}
