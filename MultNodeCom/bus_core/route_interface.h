#ifndef __ROUTE_INTERFACE_H__
#define __ROUTE_INTERFACE_H__

#include "route_protocol.h"
#include "bus_driver.h"

typedef enum {
    P_READ = 0,
    P_WRITE,
    P_NUM
}package_id_e;

typedef struct {
    uint8_t self_id;
    uint8_t src_id;
    uint8_t dst_id;
    uint8_t cmd;
    uint16_t length;
    uint8_t buffer[DATA_MAX_SIZE];
}route_data_package_t;

typedef struct{
    int (*recv)(void* self, uint8_t* buffer, uint16_t len);
    int (*send)(void* self, uint8_t dst_id, uint8_t cmd, uint8_t* buffer, uint16_t len);
    int (*bind)(void* self, bus_driver_t **bus);
}route_options_interface_i;

typedef struct {
    route_options_interface_i* interface;
    bus_driver_t **bus;
    uint8_t route_id;
    uint8_t buffer[P_NUM][MAX_PACKAGE_SIZE];
    route_data_package_t* package;
}route_item_t;

route_item_t* route_item_register(void);

#endif /* __ROUTE_INTERFACE_H__ */
