#ifndef PIPELINE_H
#define PIPELINE_H
#include <stdint.h>

typedef struct {
  uint8_t buttons;
  int8_t wheel;
} pipeline_report_t;

#ifdef __cplusplus
extern "C" {
#endif
void pipeline_init(void);
void pipeline_tick(void);
uint8_t pipeline_get_ready_report(pipeline_report_t *out);
#ifdef __cplusplus
}
#endif
#endif
