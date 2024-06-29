#ifndef __BUS_SOCKET_DEVICE_H__
#define __BUS_SOCKET_DEVICE_H__

#include "stdint.h"
#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET_LISTEN_MAX_NUM   16

typedef enum {
    NONE_TYPE = 0,
    SOCKET_CLIENT_TYPE = 1,
    SOCKET_SERVER_TYPE
}socket_type_e;

typedef struct{
    int fd;
    int server_connect_client_fd[SOCKET_LISTEN_MAX_NUM];
    struct sockaddr_in server_connect_client_sockaddr[SOCKET_LISTEN_MAX_NUM];
    uint8_t server_connect_client_idx;
    struct sockaddr_in socket_addr;
    socket_type_e type;
    char ipaddr[32];
    uint16_t port;
    uint8_t server_max_listen_num;
}bus_socket_device_t;

#ifdef __cplusplus
}
#endif
#endif /* __BUS_SOCKET_DEVICE_H__ */

