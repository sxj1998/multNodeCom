/**
 * @file bus_driver.h
 * @author shexingju (1970237065@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */
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
    struct rt_ringbuffer* write_rb;
    struct rt_ringbuffer* read_rb;
    char bus_name[BUS_NAME_NUM];
    BUS_TYPE bus_type;
    uint8_t bus_id;
    uint16_t bus_rx_buffer_size;
    uint16_t bus_tx_buffer_size;
}bus_driver_t;

int8_t bus_driver_register(bus_driver_t **bus_driver, const bus_interface_i* interface, uint16_t bus_rx_buffer_size, uint16_t bus_tx_buffer_size, const char *bus_name, BUS_TYPE bus_type, uint8_t bus_id);

int bus_init(bus_driver_t** bus);

int bus_open(bus_driver_t** bus);

int bus_close(bus_driver_t** bus);

int bus_sync_rx(bus_driver_t** bus);

int bus_sync_tx(bus_driver_t** bus);

int bus_write(bus_driver_t** bus, uint8_t *data, uint16_t length);

int bus_read(bus_driver_t** bus, uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* BUS_DRIVER_H */
