#ifndef __BUS_SERIAL_DRIVER_H__
#define __BUS_SERIAL_DRIVER_H__

#include "bus_driver.h"
#include "bus_serial_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SERIAL_NAME_NUM     32

typedef struct{
    bus_driver_t* bus_driver;
    bus_serial_device_t bus_device;
    char dev_name[SERIAL_NAME_NUM];
}bus_serial_driver_t;

static int serial_init(void *obj);
static int serial_open(void *obj);
static int serial_close(void *obj);
static int serial_write(void *obj, uint8_t *buffer, uint16_t length);
static int serial_read(void *obj, uint8_t *buffer, uint16_t length);
static int serial_write_buff(void *obj, uint8_t *buffer, uint16_t length);
static int serial_read_buff(void *obj, uint8_t *buffer, uint16_t length);
static int rb_serial_write_sync(void *obj);
static int rb_serial_read_sync(void *obj);
static int rb_serial_read_sync(void *self);


#ifdef __cplusplus
}
#endif

#endif /* __BUS_SERIAL_DRIVER_H__ */
