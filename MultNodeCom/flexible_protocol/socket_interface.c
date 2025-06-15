/* socket_interface.c */
#include "socket_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Socket上下文
typedef struct {
    int sockfd;
    struct sockaddr_in addr;
    bool is_server;
    bool connected;
} SocketContext;

// Socket初始化
static int socket_init(HardwareInterface* hw_if) {
    SocketContext* ctx = (SocketContext*)hw_if->context;
    
    ctx->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (ctx->sockfd < 0) {
        perror("socket");
        return -1;
    }
    
    if (ctx->is_server) {
        // 服务器模式
        int opt = 1;
        setsockopt(ctx->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        if (bind(ctx->sockfd, (struct sockaddr*)&ctx->addr, sizeof(ctx->addr)) < 0) {
            perror("bind");
            close(ctx->sockfd);
            return -1;
        }
        
        if (listen(ctx->sockfd, 5) < 0) {
            perror("listen");
            close(ctx->sockfd);
            return -1;
        }
        
        printf("Server listening on %s:%d\n", 
               inet_ntoa(ctx->addr.sin_addr), ntohs(ctx->addr.sin_port));
    } else {
        // 客户端模式
        if (connect(ctx->sockfd, (struct sockaddr*)&ctx->addr, sizeof(ctx->addr)) < 0) {
            perror("connect");
            close(ctx->sockfd);
            return -1;
        }
        ctx->connected = true;
        printf("Connected to server: %s:%d\n", 
               inet_ntoa(ctx->addr.sin_addr), ntohs(ctx->addr.sin_port));
    }
    
    return 0;
}

// Socket写数据
static int socket_write(HardwareInterface* hw_if, const uint8_t* data, uint16_t len) {
    SocketContext* ctx = (SocketContext*)hw_if->context;
    if (!ctx->connected) return -1;
    
    ssize_t n = send(ctx->sockfd, data, len, 0);
    if (n < 0) {
        perror("send");
        ctx->connected = false;
        return -1;
    }
    return n;
}

// Socket读数据
static int socket_read(HardwareInterface* hw_if, uint8_t* buffer, uint16_t max_len) {
    SocketContext* ctx = (SocketContext*)hw_if->context;
    if (!ctx->connected) return -1;
    
    ssize_t n = recv(ctx->sockfd, buffer, max_len, 0);
    if (n <= 0) {
        if (n == 0) {
            printf("Connection closed\n");
        } else {
            perror("recv");
        }
        ctx->connected = false;
    }
    return n;
}

// Socket关闭
static void socket_close(HardwareInterface* hw_if) {
    SocketContext* ctx = (SocketContext*)hw_if->context;
    if (ctx) {
        if (ctx->sockfd >= 0) {
            close(ctx->sockfd);
        }
        free(ctx);
    }
}

// 获取Socket描述符
int socket_interface_get_fd(HardwareInterface* hw_if) {
    SocketContext* ctx = (SocketContext*)hw_if->context;
    return ctx->sockfd;
}

// 创建Socket硬件接口
HardwareInterface* socket_interface_create(const char* ip, uint16_t port, bool is_server) {
    HardwareInterface* hw_if = malloc(sizeof(HardwareInterface));
    if (!hw_if) return NULL;
    
    SocketContext* ctx = malloc(sizeof(SocketContext));
    if (!ctx) {
        free(hw_if);
        return NULL;
    }
    
    memset(hw_if, 0, sizeof(HardwareInterface));
    memset(ctx, 0, sizeof(SocketContext));
    
    ctx->is_server = is_server;
    ctx->sockfd = -1;
    ctx->addr.sin_family = AF_INET;
    ctx->addr.sin_port = htons(port);
    
    if (ip) {
        inet_pton(AF_INET, ip, &ctx->addr.sin_addr);
    } else {
        ctx->addr.sin_addr.s_addr = INADDR_ANY;
    }
    
    // 设置操作接口
    hw_if->ops.init = socket_init;
    hw_if->ops.write = socket_write;
    hw_if->ops.read = socket_read;
    hw_if->ops.close = socket_close;
    
    hw_if->context = ctx;
    
    return hw_if;
}

// 销毁Socket硬件接口
void socket_interface_destroy(HardwareInterface* hw_if) {
    if (!hw_if) return;
    
    if (hw_if->ops.close) {
        hw_if->ops.close(hw_if);
    }
    free(hw_if);
}