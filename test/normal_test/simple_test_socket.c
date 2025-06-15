#include "bus_serial_driver.h"
#include "utilsPrintf.h"
#include "utilsAssert.h"
#include "socket.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h> 
#include <assert.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 512

int client_sockfd;
struct sockaddr_in client_serv_addr;
char clientRecvbuffer[1024];
uint8_t sendBuff[10] = {0,1,2,3,4,5,6,7,8,9};


struct sockaddr_in serv_addr;
int server_sockfd, client_server_sockfd;
struct sockaddr_in client_addr;

pthread_t thread_socket_server_sync_id, thread_socket_client_sync_id;


void handle_client(int client_sockfd) {
    char buffer_recv[BUFFER_SIZE];
    int bytes_received = recv(client_sockfd, buffer_recv, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        printf("Received %d bytes from client\n", bytes_received);
        for(int i=0; i<bytes_received; i++) {
            printf("%02x ",buffer_recv[i]);
        }
        printf("\n");
        // if (send(client_sockfd, sendBuff, 10, 0) == -1) {
        //     perror("send");
        // } 
    } else {
        perror("recv");
    }
}

void* thread_socket_server_sync(void* arg)
{
    server_sockfd = create_socket();
    printf("Client connection %d \r\n", server_sockfd);
    socket_server_address_init(&serv_addr, 8888);
    int ret_bind = socket_server_bind_socket(server_sockfd, &serv_addr);
    printf("socket_server_bind_socket ret %d \r\n", ret_bind);
    int ret_listen = socket_server_start_listening(server_sockfd, MAX_CONNECTIONS);
    printf("socket_server_start_listening ret %d \r\n", ret_listen);
    client_server_sockfd = socket_server_accept_connection(server_sockfd, &client_addr);
    printf("client_server_sockfd  %d \r\n", client_server_sockfd);
    while (1)
    {
        if (client_server_sockfd >= 0) {
            handle_client(client_server_sockfd);
        }
        usleep(1*1000);
    }
}

void* thread_socket_client_sync(void* arg)
{
    client_sockfd = create_socket();
    socket_client_address_init(&client_serv_addr, "127.0.0.1", 8888);
    connect_to_server(client_sockfd, &client_serv_addr);
    socket_write(client_sockfd, sendBuff, 10);
    
    while (1)
    {
        // int ret = socket_read(client_sockfd, clientRecvbuffer, 1024);
        // if(ret>0){
        //     printf("client recv %d bytes :", ret);
        //     for(int i=0; i<ret; i++){
        //         printf("%02x ", clientRecvbuffer[i]);
        //     }
        //     printf("\n");
        // }
        // close_socket(client_sockfd);
        socket_write(client_sockfd, sendBuff, 10);
        printf("send-----\n");
        usleep(1000*1000);
    }
}


void main(void)
{
    pthread_create(&thread_socket_server_sync_id,NULL,thread_socket_server_sync, NULL);
    pthread_detach(thread_socket_server_sync_id);
    sleep(1);
    pthread_create(&thread_socket_client_sync_id,NULL,thread_socket_client_sync, NULL);
    pthread_detach(thread_socket_client_sync_id);
    while (1)
    {
        sleep(1);
    }
    
}