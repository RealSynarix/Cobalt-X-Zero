#include "webhid.h"
#include "config.h"
#include "src/USB-Module/hid.h"
#include <string.h>

namespace cfg {
static uint8_t rx_buf[64];
static uint8_t tx_buf[64];
static bool pending_save = false;

void webhid_init() {
  memset(rx_buf, 0, sizeof(rx_buf));
  memset(tx_buf, 0, sizeof(tx_buf));
  pending_save = false;
}

bool webhid_handle_report(const uint8_t* data, uint16_t len, uint8_t* out, uint16_t* out_len) {
  if (!data || len < 4 || !out || !out_len) return false;
  uint16_t field = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
  int32_t val = (int32_t)data[2] | ((int32_t)data[3] << 8) | ((int32_t)data[4] << 16) | ((int32_t)data[5] << 24);
  uint8_t idx = data[6];
  FieldId fid = (FieldId)field;
  SetResult r = cfg_set_field(fid, val, idx);
  out[0] = (uint8_t)field;
  out[1] = (uint8_t)(field >> 8);
  out[2] = (uint8_t)r;
  out[3] = 0;
  *out_len = 4;
  if (r == SetResult::Ok) pending_save = true;
  return true;
}

void webhid_process() {
  if (pending_save) {
    cfg_save();
    pending_save = false;
  }
}
}
