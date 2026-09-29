#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif
void webhid_init(void);
int webhid_handle(uint8_t report_id,uint8_t *data,uint16_t len,uint8_t *resp,uint16_t *resp_len);
#ifdef __cplusplus
}
#endif