#ifndef SOCKET_INTERFACE_H
#define SOCKET_INTERFACE_H

#include "hardware_interface.h"
#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif

// 创建客户端socket接口
HardwareInterface* socket_client_create(const char* ip, uint16_t port);

// 创建服务器socket接口
HardwareInterface* socket_server_create(uint16_t port);

// 销毁socket接口
void socket_interface_destroy(HardwareInterface** hw_if);

#ifdef __cplusplus
}
#endif

#endif /* SOCKET_INTERFACE_H */