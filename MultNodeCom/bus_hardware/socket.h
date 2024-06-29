#ifndef SOCKET_H
#define SOCKET_H

#include "stdint.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifdef __cplusplus
extern "C" {
#endif

int create_socket(void);

int socket_write(int sockfd, uint8_t *data, uint16_t length);

int socket_read(int sockfd, char *data, uint16_t length);

void close_socket(int sockfd);

int socket_client_address_init(struct sockaddr_in *serv_addr, const char* ip_addr, uint16_t port);

int connect_to_server(int sockfd, struct sockaddr_in* serv_addr);

void socket_server_address_init(struct sockaddr_in *serv_addr, uint16_t port) ;

int socket_server_bind_socket(int sockfd, struct sockaddr_in* serv_addr);

int socket_server_start_listening(int sockfd, int max_connections);

int socket_server_accept_connection(int sockfd, struct sockaddr_in *client_addr);

#ifdef __cplusplus
}
#endif

#endif /* SOCKET_H */
