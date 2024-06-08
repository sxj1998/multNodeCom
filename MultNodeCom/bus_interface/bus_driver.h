#ifndef __BUS_DRIVER_H__
#define __BUS_DRIVER_H__

#include "bus_type.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SERIAL = 0,
    IIC,
    SPI,
    NET
}BUS_TYPE;

typedef struct 
{
    bus_interface_i* interface;
    struct rt_ringbuffer* serial_write_rb;
    struct rt_ringbuffer* serial_read_rb;
    char bus_name[BUS_NAME_NUM];
    BUS_TYPE bus_type;
    uint8_t bus_id;
    uint16_t bus_rx_buffer_size;
    uint16_t bus_tx_buffer_size;
}bus_driver_t;

int8_t bus_driver_register(bus_driver_t *bus_driver, const bus_interface_i* interface, uint16_t bus_rx_buffer_size, uint16_t bus_tx_buffer_size, const char *bus_name, BUS_TYPE bus_type, uint8_t bus_id);

#ifdef __cplusplus
}
#endif

#endif /* BUS_DRIVER_H */
