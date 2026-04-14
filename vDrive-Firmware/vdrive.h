#ifndef VDRIVE_H
#define VDRIVE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
    #endif

    void vdrive_init(void);
    uint8_t vdrive_enabled(void);

    #ifdef __cplusplus
}
#endif

#endif
