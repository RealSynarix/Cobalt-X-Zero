#include "vdrive.h"
#include <USBMSC.h>
#include <string.h>

static const uint32_t SECTOR_COUNT = 141312;
static const uint32_t SECTOR_SIZE = 512;

static USBMSC usb_msc;

static const char kConfigIni[] = "[Mouse]\r\nCPI=800\r\n";

static int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize) {
  uint8_t* b = (uint8_t*)buffer;
  memset(b, 0, bufsize);

  if (lba == 0) {
    b[0] = 0xEB;
    b[1] = 0x3C;
    b[2] = 0x90;
    memcpy(b + 3, "MSDOS5.0", 8);
    b[11] = 0x00;
    b[12] = 0x02;
    b[13] = 0x40;
    b[14] = 0x01;
    b[15] = 0x00;
    b[16] = 0x02;
    b[17] = 0x10;
    b[18] = 0x00;
    b[19] = 0x00;
    b[20] = 0x00;
    b[21] = 0xF8;
    b[22] = 0x10;
    b[23] = 0x00;
    b[24] = 0x01;
    b[25] = 0x00;
    b[26] = 0x01;
    b[27] = 0x00;
    b[32] = 0x00;
    b[33] = 0x28;
    b[34] = 0x02;
    b[35] = 0x00;
    b[36] = 0x80;
    b[37] = 0x00;
    b[38] = 0x29;
    b[39] = 0x78;
    b[40] = 0x56;
    b[41] = 0x34;
    b[42] = 0x12;
    memcpy(b + 43, "COBALT-X  ", 11);
    memcpy(b + 54, "FAT16   ", 8);
    b[510] = 0x55;
    b[511] = 0xAA;
  } else if (lba == 1 || lba == 17) {
    b[0] = 0xF8;
    b[1] = 0xFF;
    b[2] = 0xFF;
    b[3] = 0xFF;
    b[4] = 0xFF;
    b[5] = 0xFF;
  } else if (lba == 33) {
    b[0] = 'C';
    b[1] = 'O';
    b[2] = 'N';
    b[3] = 'F';
    b[4] = 'I';
    b[5] = 'G';
    b[6] = ' ';
    b[7] = ' ';
    b[8] = 'I';
    b[9] = 'N';
    b[10] = 'I';
    b[11] = 0x20;
    b[26] = 0x02;
    b[27] = 0x00;
    uint32_t fsize = strlen(kConfigIni);
    b[28] = (uint8_t)fsize;
    b[29] = (uint8_t)(fsize >> 8);
    b[30] = (uint8_t)(fsize >> 16);
    b[31] = (uint8_t)(fsize >> 24);
  } else if (lba == 34) {
    memcpy(b, kConfigIni, strlen(kConfigIni));
  }
  return (int32_t)bufsize;
}

static int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
  (void)lba;
  (void)buffer;
  return (int32_t)bufsize;
}

static void msc_flush_cb(void) {}

void vdrive_init(void) {
  usb_msc.setID("Synarix", "Cobalt-X Config", "1.0");
  usb_msc.setCapacity(SECTOR_COUNT, SECTOR_SIZE);
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
  usb_msc.begin();
}