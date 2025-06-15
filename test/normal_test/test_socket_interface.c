#include "socket_interface.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define PORT 8888

void* server_thread(void* arg) {
    printf("[SERVER] Starting...\n");
    HardwareInterface* server = socket_server_create(PORT);
    if (!server) {
        printf("Server creation failed\n");
        return NULL;
    }
    
    uint8_t buffer[128];
    while (1) {
        int len = server->ops.read(server, buffer, sizeof(buffer));
        if (len > 0) {
            printf("[SERVER] Received %d bytes: ", len);
            for (int i = 0; i < len; i++) {
                printf("%02X ", buffer[i]);
            }
            printf("\n");
            
            // 回显数据
            if (server->ops.write(server, buffer, len) != len) {
                perror("server write failed");
            }
        } else if (len < 0) {
            break;
        }
        usleep(10000);
    }
    
    socket_interface_destroy(&server);
    return NULL;
}

void* client_thread(void* arg) {
    printf("[CLIENT] Waiting 1 second for server to start...\n");
    sleep(1);
    
    printf("[CLIENT] Connecting...\n");
    HardwareInterface* client = socket_client_create("127.0.0.1", PORT);
    if (!client) {
        printf("Client creation failed\n");
        return NULL;
    }
    
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t recv_buffer[128];
    
    for (int i = 0; i < 5; i++) {
        // 发送测试数据
        printf("[CLIENT] Sending test data...\n");
        if (client->ops.write(client, test_data, sizeof(test_data)) < 0) {
            perror("client write failed");
            break;
        }
        
        // 接收响应
        printf("[CLIENT] Waiting for response...\n");
        int len = client->ops.read(client, recv_buffer, sizeof(recv_buffer));
        if (len > 0) {
            printf("[CLIENT] Received %d bytes: ", len);
            for (int j = 0; j < len; j++) {
                printf("%02X ", recv_buffer[j]);
            }
            printf("\n");
        } else if (len < 0) {
            break;
        }
        
        sleep(1);
    }
    
    socket_interface_destroy(&client);
    printf("[CLIENT] Exiting\n");
    return NULL;
}

int main() {
    pthread_t srv_tid, cli_tid;
    
    printf("Starting server thread...\n");
    pthread_create(&srv_tid, NULL, server_thread, NULL);
    
    printf("Starting client thread...\n");
    pthread_create(&cli_tid, NULL, client_thread, NULL);
    
    pthread_join(srv_tid, NULL);
    pthread_join(cli_tid, NULL);
    
    printf("Test completed\n");
    return 0;
}