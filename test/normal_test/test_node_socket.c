#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "socket_interface.h"
#include "router.h"
#include "xlog.h"

// 节点 ID 定义
#define NODE1_ID 0x01
#define NODE2_ID 0x02

// 自定义命令定义
#define CMD_PING  0x10
#define CMD_PONG  0x11
#define CMD_DATA  0x20

// 全局变量
Node node1;
Node node2;
HardwareInterface* node1_server = NULL;
HardwareInterface* node2_client = NULL;
pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t init_cond = PTHREAD_COND_INITIALIZER;
bool server_ready = false;

/* 自定义日志处理器实现 */
void custom_log_handler(LogLevel level, const char* message, void* user_data) {
    // 所有日志都输出到控制台 (带颜色)
    switch (level) {
        case LOG_LEVEL_DEBUG:
            printf("\033[0;36m"); // 青色
            break;
        case LOG_LEVEL_INFO:
            printf("\033[0;32m"); // 绿色
            break;
        case LOG_LEVEL_WARNING:
            printf("\033[0;33m"); // 黄色
            break;
        case LOG_LEVEL_ERROR:
            printf("\033[0;31m"); // 红色
            break;
        case LOG_LEVEL_CRITICAL:
            printf("\033[1;31m"); // 粗体红色
            break;
        default:
            printf("\033[0m"); // 默认
    }
    printf("%s\033[0m", message); // 重置颜色
}

// 数据包处理函数
void node1_packet_handler(Node* node, protocol_t* packet) {
    printf("Node1 received packet: cmd=0x%02X, src=0x%02X, len=%u\n", 
           packet->cmd, packet->src_id, PROTO_NTOHS(packet->length));
    
    switch (packet->cmd) {
        case CMD_PING:
            printf("Node1: Received PING from Node2\n");
            // 发送 PONG 响应
            uint8_t pong_data[] = {0xAA, 0xBB, 0xCC};
            node_send_packet(&node1, NODE2_ID, CMD_PONG, sizeof(pong_data), pong_data);
            break;
            
        case CMD_DATA:
            printf("Node1: Received DATA from Node2\n");
            // 处理数据...
            break;
            
        default:
            printf("Node1: Unknown command 0x%02X\n", packet->cmd);
    }
}

void node2_packet_handler(Node* node, protocol_t* packet) {
    printf("Node2 received packet: cmd=0x%02X, src=0x%02X, len=%u\n", 
           packet->cmd, packet->src_id, PROTO_NTOHS(packet->length));
    
    switch (packet->cmd) {
        case CMD_PONG:
            printf("Node2: Received PONG from Node1\n");
            break;
            
        case CMD_DATA:
            printf("Node2: Received DATA from Node1\n");
            // 处理数据...
            break;
            
        default:
            printf("Node2: Unknown command 0x%02X\n", packet->cmd);
    }
}

// 读取线程函数
void* read_thread_func(void* arg) {
    Node* node = (Node*)arg;
    uint8_t buffer[128];
    
    while (1) {
        for (int i = 0; i < node->interface_count; i++) {
            int bytes_read = node->interfaces[i].hw_if->ops.read(
                node->interfaces[i].hw_if, 
                buffer, 
                sizeof(buffer)
            );
            
            if (bytes_read > 0) {
                // 将接收到的字节传递给解析器
                for (int j = 0; j < bytes_read; j++) {
                    node_receive_byte(node, i, buffer[j]);
                }
            } else if (bytes_read < 0) {
                perror("Error reading from interface");
            }
        }
        usleep(1000); // 短暂休眠避免CPU占用过高
    }
    return NULL;
}

// 服务器初始化线程函数
void* server_init_thread(void* arg) {
    printf("Server thread: Creating Node1 server interface on port 12345...\n");
    node1_server = socket_server_create(12345);
    
    if (!node1_server) {
        fprintf(stderr, "Server thread: Failed to create Node1 server\n");
        return NULL;
    }
    
    printf("Server thread: Server created successfully\n");
    
    // 通知主线程服务器已准备好
    pthread_mutex_lock(&init_mutex);
    server_ready = true;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_mutex);
    
    // 初始化节点
    node_init(&node1, NODE1_ID, 2);
    node_set_packet_handler(&node1, node1_packet_handler);
    node_add_interface(&node1, node1_server);
    printf("Server thread: Node1 initialized\n");
    
    // 启动读取线程
    pthread_t read_thread;
    pthread_create(&read_thread, NULL, read_thread_func, &node1);
    printf("Server thread: Read thread started\n");
    
    // 保持线程运行
    while (1) {
        sleep(1);
    }
    
    return NULL;
}

// 客户端初始化线程函数
void* client_init_thread(void* arg) {
    // 等待服务器准备好
    pthread_mutex_lock(&init_mutex);
    while (!server_ready) {
        printf("Client thread: Waiting for server to be ready...\n");
        pthread_cond_wait(&init_cond, &init_mutex);
    }
    pthread_mutex_unlock(&init_mutex);
    
    printf("Client thread: Creating Node2 client interface...\n");
    node2_client = socket_client_create("127.0.0.1", 12345);
    
    if (!node2_client) {
        fprintf(stderr, "Client thread: Failed to create Node2 client\n");
        return NULL;
    }
    
    printf("Client thread: Client created successfully\n");
    
    // 初始化节点
    node_init(&node2, NODE2_ID, 2);
    node_set_packet_handler(&node2, node2_packet_handler);
    node_add_interface(&node2, node2_client);
    printf("Client thread: Node2 initialized\n");
    
    // 启动读取线程
    pthread_t read_thread;
    pthread_create(&read_thread, NULL, read_thread_func, &node2);
    printf("Client thread: Read thread started\n");
    
    // 保持线程运行
    while (1) {
        sleep(1);
    }
    
    return NULL;
}

// 通信测试线程函数
void* communication_thread(void* arg) {
    // 等待客户端准备好
    sleep(2);
    
    uint8_t ping_data[] = {0x01, 0x02, 0x03};
    uint8_t data[256];
    
    for (int i = 0; i < sizeof(data); i++) {
        data[i] = i % 256;
    }
    
    while (1) {
        printf("\nSending PING from Node2 to Node1\n");
        node_send_packet(&node2, NODE1_ID, CMD_PING, sizeof(ping_data), ping_data);
        
        sleep(1);
        
        printf("\nSending DATA from Node1 to Node2\n");
        node_send_packet(&node1, NODE2_ID, CMD_DATA, sizeof(data), data);
        
        sleep(1);
    }
    
    return NULL;
}

int main() {
    // 初始化日志系统
    log_init(LOG_LEVEL_DEBUG, 0, custom_log_handler, NULL);
    printf("=== Starting Multi-Threaded Node Communication Test ===\n");
    
    // 创建服务器初始化线程
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_init_thread, NULL);
    printf("Main thread: Server initialization thread started\n");
    
    // 创建客户端初始化线程
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, client_init_thread, NULL);
    printf("Main thread: Client initialization thread started\n");
    
    // 创建通信测试线程
    pthread_t comm_thread;
    pthread_create(&comm_thread, NULL, communication_thread, NULL);
    printf("Main thread: Communication thread started\n");
    
    // 等待所有线程完成（实际上它们会一直运行）
    pthread_join(server_thread, NULL);
    pthread_join(client_thread, NULL);
    pthread_join(comm_thread, NULL);
    
    // 清理资源（实际上永远不会到达这里）
    printf("\nCleaning up...\n");
    node_destroy(&node1);
    node_destroy(&node2);
    socket_interface_destroy(&node1_server);
    socket_interface_destroy(&node2_client);
    
    printf("Program completed\n");
    return 0;
}