#ifndef SPI_H
#define SPI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  void spi_init(void);
  void sensor_read_motion(int8_t *dx, int8_t *dy);

#ifdef __cplusplus
}
#endif

#endif