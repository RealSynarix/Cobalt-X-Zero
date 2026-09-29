#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"{
#endif
extern uint8_t feature_report_buf[65];
void hid_init(void);
void hid_task(void);
void hid_set_report(uint8_t type,uint8_t id,const uint8_t *data,uint16_t len);
uint16_t hid_get_report(uint8_t type,uint8_t id,uint8_t *out,uint16_t max);
void hid_send(uint8_t buttons,int8_t wheel,int16_t dx,int16_t dy);
void hid_type_program_me(void);
uint8_t hid_slot(void);
void hid_service(void);
void hid_pio_trigger(uint8_t event);
uint32_t hid_last_activity_ms(void);
#ifdef __cplusplus
}
#endif
