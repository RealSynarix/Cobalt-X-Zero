#pragma once
#include <stdint.h>

namespace cfg {
void webhid_init();
void webhid_process();
bool webhid_handle_report(const uint8_t* data, uint16_t len, uint8_t* out, uint16_t* out_len);
}
