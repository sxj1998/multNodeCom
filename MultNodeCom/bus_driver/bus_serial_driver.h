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

static int serial_init(void *self);
static int serial_open(void *self);
static int serial_close(void *self);
static int serial_write(void *self, uint8_t *buffer, uint16_t length);
static int serial_read(void *self, uint8_t *buffer, uint16_t length);
static int serial_write_buff(void *self, uint8_t *buffer, uint16_t length);
static int serial_read_buff(void *self, uint8_t *buffer, uint16_t length);
static int rb_serial_write_sync(void *self);
static int rb_serial_read_sync(void *self);
static int rb_serial_read_sync(void *self);

bus_serial_driver_t* bus_serial_driver_register(const char* dev_name);


#ifdef __cplusplus
}
#endif

#endif /* __BUS_SERIAL_DRIVER_H__ */
