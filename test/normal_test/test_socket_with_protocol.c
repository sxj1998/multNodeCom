#include "bus_serial_driver.h"
#include "protocol.h"
#include "utilsPrintf.h"
#include "utilsAssert.h"
#include "socket.h"
#include "xlog.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h> 
#include <assert.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>

#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 512



char clientRecvbuffer[1024];
uint8_t sendBuff[10] = {0,1,2,3,4,5,6,7,8,9};

pthread_t thread_socket_server_sync_id, thread_socket_client_sync_id;

// 全局退出标志
volatile int exit_flag = 0;

void custom_log_handler(LogLevel level, const char* message, void* user_data) {
    // 从用户数据获取日志文件名
    const char* filename = (const char*)user_data;
    
    // 仅记录警告及以上级别的日志到文件
    if (level >= LOG_LEVEL_WARNING && filename != NULL) {
        FILE* logfile = fopen(filename, "a");
        if (logfile) {
            fprintf(logfile, "%s", message);
            fclose(logfile);
        }
    }
    
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

// 信号处理函数
void sigint_handler(int sig) {
    exit_flag = 1;
}

void packet_handler(void* packet, void* user_data) {
    protocol_t* p = (protocol_t*)packet;
    uint16_t length = PROTO_NTOHS(p->length);
    printf("Received packet:index=%u cmd=%u, len=%u\n",p->index, p->cmd, length);
    
    // 处理包数据...
    printf("Data: ");
    for (int i = 0; i < length; i++) {
        printf("%02X ",p->data[i]);
    }
    printf("\n");
    
    proto_packet_free(&packet);
}

void handle_client(int client_sockfd, void* userdata) {
    proto_parser_t* parser = (proto_parser_t*)userdata;
    char buffer_recv[BUFFER_SIZE];
    int bytes_received = recv(client_sockfd, buffer_recv, BUFFER_SIZE, 0);
    for(int i=0; i<bytes_received; i++)
    {
        PARSE_STATUS status = proto_packet_parse(parser, buffer_recv[i]);
        if (status == PARSE_OK) {
            printf("Packet parsed successfully\n");
        } else if (status != PARSE_INCOMPLETE) {
            printf("Parse error: %d\n", status);
            break;
        }
    }

    // if (bytes_received > 0) {
    //     printf("Received %d bytes from client\n", bytes_received);
    //     for(int i=0; i<bytes_received; i++) {
    //         printf("%02x ",buffer_recv[i]);
    //     }
    //     printf("\n");
    // } else {
    //     perror("recv");
    // }
}

void* thread_socket_server_sync(void* arg)
{
    struct sockaddr_in serv_addr;       // 服务器地址结构
    int server_sockfd, client_server_sockfd; // 套接字描述符
    struct sockaddr_in client_addr;      // 客户端地址结构

    proto_parser_t parser;
    proto_parser_init(&parser);
    proto_parser_set_callback(&parser, packet_handler, NULL);

            // 初始化日志系统
    log_init(LOG_LEVEL_INFO, 
            0, 
            custom_log_handler, NULL);
    
    // 1. 创建服务器套接字
    server_sockfd = create_socket();
    printf("Client connection %d \r\n", server_sockfd);
    
    // 2. 初始化服务器地址结构（端口8888）
    socket_server_address_init(&serv_addr, 8888);
    
    // 3. 绑定套接字到服务器地址
    int ret_bind = socket_server_bind_socket(server_sockfd, &serv_addr);
    printf("socket_server_bind_socket ret %d \r\n", ret_bind);
    
    // 4. 开始监听连接请求
    int ret_listen = socket_server_start_listening(server_sockfd, MAX_CONNECTIONS);
    printf("socket_server_start_listening ret %d \r\n", ret_listen);

    // 5. 接受客户端连接
    client_server_sockfd = socket_server_accept_connection(server_sockfd, &client_addr);
    printf("client_server_sockfd  %d \r\n", client_server_sockfd);
    
    // 6. 主循环：处理客户端数据
    while (!exit_flag)
    {
        if (client_server_sockfd >= 0) {
            handle_client(client_server_sockfd, (void*)&parser); // 处理客户端数据
        }
        usleep(1*1000); // 每次循环后休眠1毫秒
    }
    close(client_server_sockfd);  // 关闭客户端套接字
    close(server_sockfd);         // 关闭服务器套接字
    proto_parser_destroy(&parser); // 清理解析器（假设有该函数）
    return NULL;
}

void* thread_socket_client_sync(void* arg)
{
    struct sockaddr_in client_serv_addr; // 服务器地址结构

    proto_parser_t parser;
    proto_parser_init(&parser);
    proto_parser_set_callback(&parser, packet_handler, NULL);

            // 初始化日志系统
    log_init(LOG_LEVEL_DEBUG, 
            0, 
            custom_log_handler, NULL);

    // 1. 创建客户端套接字
    int client_sockfd = create_socket();
    
    // 2. 初始化服务器地址（IP: 127.0.0.1, 端口: 8888）
    socket_client_address_init(&client_serv_addr, "127.0.0.1", 8888);
    
    // 3. 连接到服务器
    connect_to_server(client_sockfd, &client_serv_addr);
    
    // 4. 发送初始测试数据
    
    // 5. 主循环：每秒发送一次测试数据
    uint8_t payload[] = {0x11, 0x5B, 0x5A, 0xA5, 0x55};
    while (!exit_flag)
    {
        // uint8_t i=0;
        // payload[4] = i++;
        protocol_t* packet = proto_create_packet(0x01, 0x02, 0x03, 5, payload);
        if (!packet) {
            fprintf(stderr, "Failed to create packet\n");
            break;
        }
        // size_t total_size = GET_PACKET_LEN(sizeof(payload));
        size_t total_size = proto_get_result_length(packet);
        
        socket_write(client_sockfd, (void*)packet, total_size); // 发送10字节数据
        proto_packet_free((void**)&packet);

        printf("send %ld bytes----->\n", total_size); // 打印发送提示
        usleep(100*1000); // 休眠1秒（1000毫秒）
    }
    close(client_sockfd);  // 关闭客户端套接字
    proto_parser_destroy(&parser); // 清理解析器（假设有该函数）
    return NULL;
}


int main(void)
{

    signal(SIGINT, sigint_handler);
    pthread_create(&thread_socket_server_sync_id,NULL,thread_socket_server_sync, NULL);
    // pthread_detach(thread_socket_server_sync_id);
    sleep(1);
    pthread_create(&thread_socket_client_sync_id,NULL,thread_socket_client_sync, NULL);
    // pthread_detach(thread_socket_client_sync_id);
    while (!exit_flag){
        sleep(1);
    }
        // 等待子线程结束
    pthread_join(thread_socket_server_sync_id, NULL);
    pthread_join(thread_socket_client_sync_id, NULL);
    return 0;
}