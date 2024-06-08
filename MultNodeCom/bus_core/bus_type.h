#ifndef __BUS_TYPE_H__
#define __BUS_TYPE_H__

#include <stdint.h>
#include "ringbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  BUS_NAME_NUM   32

typedef struct 
{
    int (*init)(void *self);
    int (*open)(void *self);
    int (*close)(void *self);
    int (*sync_rx)(void *self);
    int (*sync_tx)(void *self);
    int (*write)(void *self, uint8_t *data, uint16_t length);
    int (*read)(void *self, uint8_t *data, uint16_t length);
}bus_interface_i;

#ifdef __cplusplus
}
#endif

#endif /* __BUS_TYPE_H__ */
