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
    bus_device_t bus_device;
    char dev_name[SERIAL_NAME_NUM];
}bus_serial_driver_t;

bus_serial_driver_t* bus_serial_driver_register(const char* dev_name, uint8_t bus_id);


#ifdef __cplusplus
}
#endif

#endif /* __BUS_SERIAL_DRIVER_H__ */
