#include "hid.h"
#include "../Config-Module/webhid.h"
#include <Arduino.h>
#include <Mouse.h>
#include <string.h>

#define REPORT_ID   2
#define REPORT_SIZE 64

uint8_t feature_report_buf[REPORT_SIZE + 1] = {0};

static void reset_report(void)
{
  memset(feature_report_buf, 0, sizeof(feature_report_buf));
  feature_report_buf[0] = REPORT_ID;
}

static int report_set(uint8_t id, uint8_t *data, uint16_t len)
{
  uint8_t resp[REPORT_SIZE] = {0};
  uint16_t rlen = 0;

  if (len > REPORT_SIZE) {
    data++;
    len--;
  }
  if (len > REPORT_SIZE) {
    len = REPORT_SIZE;
  }

  if (webhid_handle(id, data, len, resp, &rlen) != 0) {
    resp[0] = len > 0 ? data[0] : 0;
    resp[1] = len > 1 ? data[1] : 0;
    resp[4] = 1;
    rlen = 5;
  }
  if (rlen > REPORT_SIZE) {
    rlen = REPORT_SIZE;
  }

  reset_report();
  memcpy(feature_report_buf + 1, resp, rlen);
  return 0;
}

static int report_get(uint8_t id, uint8_t *data, uint16_t len)
{
  (void)id;
  if (len > sizeof(feature_report_buf)) {
    len = sizeof(feature_report_buf);
  }
  memcpy(data, feature_report_buf, len);
  return len;
}

extern "C" {

int CUSTOM_HID_ReceiveReport(uint8_t id, uint8_t *d, uint16_t l) { return report_set(id, d, l); }
int CUSTOM_HID_SetReport(uint8_t id, uint8_t *d, uint16_t l)     { return report_set(id, d, l); }
int CUSTOM_HID_GetReport(uint8_t id, uint8_t *d, uint16_t l)     { return report_get(id, d, l); }

int HID_Custom_ReceiveReport(uint8_t id, uint8_t *d, uint16_t l) { return report_set(id, d, l); }
int HID_Custom_SetReport(uint8_t id, uint8_t *d, uint16_t l)     { return report_set(id, d, l); }
int HID_Custom_GetReport(uint8_t id, uint8_t *d, uint16_t l)     { return report_get(id, d, l); }

int HID_ReceiveReport(uint8_t id, uint8_t *d, uint16_t l)        { return report_set(id, d, l); }
int HID_SetFeatureReport(uint8_t id, uint8_t *d, uint16_t l)     { return report_set(id, d, l); }
int HID_GetFeatureReport(uint8_t id, uint8_t *d, uint16_t l)     { return report_get(id, d, l); }

}

void hid_init(void)
{
  webhid_init();
  reset_report();
  Mouse.begin();
}
