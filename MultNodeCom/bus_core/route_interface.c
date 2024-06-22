/**
 * @file route_interface.c
 * @author shexingju (1970237065@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "route_interface.h"
#include "bus_driver.h"
#include "utilsAssert.h"
#include "utilsPrintf.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

MODULE_TAG("ROUTE"); 

static int route_data_recv(void* obj, uint8_t* buffer, uint16_t len);
static int route_data_send(void* self, uint8_t src_id, uint8_t dst_id, uint8_t cmd, uint8_t* buffer, uint16_t len);
static int route_item_bind(void* self, bus_driver_t **bus);

static const route_options_interface_i route_opt_interface = {
    .bind = route_item_bind,
    .recv = route_data_recv,
    .send = route_data_send
};

// static route_data_package_t route_data_package[P_NUM] = {0};

route_item_t* route_item_register(void)
{
    route_item_t* item = NULL;
    item = malloc(sizeof(route_item_t));
    item->route_id = 0;
    item->package = malloc(P_NUM*sizeof(route_data_package_t));
    item->interface = (route_options_interface_i*)&route_opt_interface;
    item->bus = NULL;
    return item;
}

static int route_data_recv(void* self, uint8_t* buffer, uint16_t len)
{
    route_item_t* item = (route_item_t *)self;
    bus_driver_t* bus_driver = *(item->bus);
    bus_interface_i* interface = bus_driver->interface;
    int length = interface->read((void*)item->bus, (uint8_t*)&item->buffer[P_READ], len);
    if(length > 0) {
        return length;
    }
    return 0;
}

static int route_data_send(void* self, uint8_t src_id, uint8_t dst_id, uint8_t cmd, uint8_t* buffer, uint16_t len)
{
    route_item_t *item = (route_item_t *)self;
    bus_driver_t* bus_driver = *(item->bus);
    bus_interface_i* interface = bus_driver->interface;

    uint16_t temp_length = 0;
    pack_route_data((uint8_t*)&item->buffer[P_WRITE], &temp_length, buffer, len, src_id, dst_id, cmd);
    int length = interface->write((void*)item->bus, (uint8_t*)&item->buffer[P_WRITE], temp_length);
    TI_DEBUG("ROUTE TX ID %02x: dst_id 0x%02x cmd 0x%02x",src_id,dst_id,cmd);
    TI_DEBUG("ROUTE TX BUS %s id %d",bus_driver->bus_name, bus_driver->bus_id);
    if(length > 0) {
        return length;
    }
    return 0;
}

static int route_item_bind(void* self, bus_driver_t **bus)
{
    utils_assert(bus);
    bus_interface_i* interface = (bus_interface_i*)(*bus)->interface;
    route_item_t* route_item = (route_item_t*)self;
    route_item->bus = bus;
    route_item->route_id = (*bus)->bus_id;
    TI_DEBUG("bind route_item to bus_driver %s", (*bus)->bus_name);
    return 0;
}