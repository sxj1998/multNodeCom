#include "bus_driver.h"
#include "ringbuffer.h"
#include "bus_type.h"
#include "utilsPrintf.h"
#include "utilsAssert.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

MODULE_TAG("BUS_DRIVER"); 

int8_t bus_driver_register(bus_driver_t *bus_driver, const bus_interface_i* interface, uint16_t bus_rx_buffer_size, uint16_t bus_tx_buffer_size, const char *bus_name, BUS_TYPE bus_type, uint8_t bus_id) {
    if(interface == NULL || bus_driver == NULL || bus_rx_buffer_size == 0 || bus_tx_buffer_size == 0){
        TI_DEBUG(" %s bus driver register is null !", bus_name);
        return -1;   
    }

    bus_driver_t* bus = NULL;
    bus = malloc(sizeof(bus_driver_t));

    bus->interface = (bus_interface_i *)interface;
    strncpy(bus_driver->bus_name, bus_name, sizeof(bus_driver->bus_name) - 1);
    bus_driver->bus_name[sizeof(bus_driver->bus_name) - 1] = '\0'; // 确保字符串以空字符结尾
    bus_driver->bus_type = bus_type;
    bus_driver->bus_id = bus_id;
    bus_driver->bus_rx_buffer_size = bus_rx_buffer_size;
    bus_driver->bus_tx_buffer_size = bus_tx_buffer_size;

    bus->serial_write_rb = rt_ringbuffer_create(bus_driver->bus_tx_buffer_size);
    if(bus->serial_write_rb == NULL){
        TI_DEBUG("%s malloc ring buffer failed !\n", bus_name);
        goto free_bus_driver;
    }

    bus->serial_read_rb = rt_ringbuffer_create(bus_driver->bus_rx_buffer_size);
    if(bus->serial_read_rb == NULL){
        TI_DEBUG("%s malloc ring buffer failed !\n", bus_name);
        goto free_ringbuffer_tx;
    }

    bus_driver = bus;
    TI_DEBUG("Bus driver %s with type %d and id %d\n", bus_driver->bus_name, bus_type, bus_id);
    return 0;

free_ringbuffer_rx:
    rt_ringbuffer_destroy(bus->serial_read_rb);

free_ringbuffer_tx:
    rt_ringbuffer_destroy(bus->serial_write_rb);

free_bus_driver:
    free(bus);

    return -1;
}