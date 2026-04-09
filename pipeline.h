#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  void pipeline_init(void);
  void pipeline_tick(void);
  uint8_t pipeline_dequeue(uint8_t *out);

#ifdef __cplusplus
}
#endif

#endif