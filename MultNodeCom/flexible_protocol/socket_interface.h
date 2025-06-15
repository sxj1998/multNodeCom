/* socket_interface.h */
#ifndef SOCKET_INTERFACE_H
#define SOCKET_INTERFACE_H

#include "router.h"
#include "hardware_interface.h"
#include <stdint.h>

// 创建Socket硬件接口
HardwareInterface* socket_interface_create(const char* ip, uint16_t port, bool is_server);

// 销毁Socket硬件接口
void socket_interface_destroy(HardwareInterface* hw_if);

// 获取Socket描述符
int socket_interface_get_fd(HardwareInterface* hw_if);

#endif // SOCKET_INTERFACE_H