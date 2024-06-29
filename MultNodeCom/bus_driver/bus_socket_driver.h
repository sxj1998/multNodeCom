/**
 * @file bus_socket_driver.h
 * @author shexingju (1970237065@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-06-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __BUS_SOCKET_DRIVER_H__
#define __BUS_SOCKET_DRIVER_H__

#include "bus_driver.h"
#include "bus_socket_device.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SOCKET_NAME_NUM         32

typedef struct{
    bus_driver_t* bus_driver;
    bus_socket_device_t bus_device;
    char dev_name[SOCKET_NAME_NUM];
}bus_socket_driver_t;

bus_socket_driver_t* bus_socket_driver_register(const char* dev_name, uint8_t bus_id, char* socket_ip, uint16_t port, socket_type_e type);


#ifdef __cplusplus
}
#endif

#endif /* __BUS_SOCKET_DRIVER_H__ */

