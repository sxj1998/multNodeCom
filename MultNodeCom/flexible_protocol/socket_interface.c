#include "socket_interface.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
    int is_server;
    int client_sockfd;  // 服务器专用
} SocketContext;

static int socket_interface_init(HardwareInterface* hw_if) {
    SocketContext* ctx = (SocketContext*)hw_if->context;
    
    if (ctx->is_server) {
        // 服务器初始化
        ctx->sockfd = create_socket();
        if (ctx->sockfd < 0) {
            perror("create_socket (server)");
            return -1;
        }
        
        // 设置SO_REUSEADDR选项
        int opt = 1;
        if (setsockopt(ctx->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt");
            close_socket(ctx->sockfd);
            return -1;
        }
        
        // 绑定并监听
        if (socket_server_bind_socket(ctx->sockfd, &ctx->addr) < 0) {
            close_socket(ctx->sockfd);
            return -1;
        }
        
        if (socket_server_start_listening(ctx->sockfd, 5) < 0) {
            close_socket(ctx->sockfd);
            return -1;
        }
        
        printf("[SOCKET] Server listening on port %d\n", ntohs(ctx->addr.sin_port));
        return 0;
    } else {
        // 客户端初始化
        ctx->sockfd = create_socket();
        if (ctx->sockfd < 0) {
            perror("create_socket (client)");
            return -1;
        }
        
        if (connect_to_server(ctx->sockfd, &ctx->addr) < 0) {
            close_socket(ctx->sockfd);
            return -1;
        }
        
        printf("[SOCKET] Client connected to %s:%d\n", 
               inet_ntoa(ctx->addr.sin_addr), ntohs(ctx->addr.sin_port));
        return 0;
    }
}

static int socket_interface_write(HardwareInterface* hw_if, const uint8_t* data, uint16_t len) {
    SocketContext* ctx = (SocketContext*)hw_if->context;
    int sock = ctx->is_server ? ctx->client_sockfd : ctx->sockfd;
    
    if (sock < 0) {
        fprintf(stderr, "Invalid socket fd: %d\n", sock);
        return -1;
    }
    
    int result = socket_write(sock, (uint8_t*)data, len);
    if (result < 0) {
        perror("socket_write");
    }
    return result;
}

static int socket_interface_read(HardwareInterface* hw_if, uint8_t* buffer, uint16_t max_len) {
    SocketContext* ctx = (SocketContext*)hw_if->context;
    int sock = ctx->is_server ? ctx->client_sockfd : ctx->sockfd;
    
    if (sock < 0) {
        fprintf(stderr, "Invalid socket fd: %d\n", sock);
        return -1;
    }
    
    int result = socket_read(sock, (char*)buffer, max_len);
    if (result < 0) {
        perror("socket_read");
    }
    return result;
}

static void socket_interface_close(HardwareInterface* hw_if) {
    if (!hw_if) return;
    
    SocketContext* ctx = (SocketContext*)hw_if->context;
    if (!ctx) return;
    
    if (ctx->is_server) {
        if (ctx->client_sockfd >= 0) close_socket(ctx->client_sockfd);
        if (ctx->sockfd >= 0) close_socket(ctx->sockfd);
    } else {
        if (ctx->sockfd >= 0) close_socket(ctx->sockfd);
    }
}

static HardwareInterfaceOps socket_interface_ops = {
    .init = socket_interface_init,
    .write = socket_interface_write,
    .read = socket_interface_read,
    .close = socket_interface_close
};

HardwareInterface* socket_client_create(const char* ip, uint16_t port) {
    SocketContext* ctx = malloc(sizeof(SocketContext));
    if (!ctx) return NULL;
    
    memset(ctx, 0, sizeof(SocketContext));
    ctx->is_server = 0;
    
    // 初始化客户端地址
    if (socket_client_address_init(&ctx->addr, ip, port) < 0) {
        free(ctx);
        return NULL;
    }
    
    HardwareInterface* hw_if = malloc(sizeof(HardwareInterface));
    if (!hw_if) {
        free(ctx);
        return NULL;
    }
    
    hw_if->context = ctx;
    hw_if->ops = socket_interface_ops;
    
    // 初始化客户端连接
    if (hw_if->ops.init(hw_if)) {
        fprintf(stderr, "Client init failed\n");
        free(ctx);
        free(hw_if);
        return NULL;
    }
    
    return hw_if;
}

HardwareInterface* socket_server_create(uint16_t port) {
    SocketContext* ctx = malloc(sizeof(SocketContext));
    if (!ctx) return NULL;
    
    memset(ctx, 0, sizeof(SocketContext));
    ctx->is_server = 1;
    ctx->client_sockfd = -1;
    
    // 初始化服务器地址
    socket_server_address_init(&ctx->addr, port);
    
    HardwareInterface* hw_if = malloc(sizeof(HardwareInterface));
    if (!hw_if) {
        free(ctx);
        return NULL;
    }
    
    hw_if->context = ctx;
    hw_if->ops = socket_interface_ops;
    
    // 初始化服务器
    if (hw_if->ops.init(hw_if)) {
        fprintf(stderr, "Server init failed\n");
        free(ctx);
        free(hw_if);
        return NULL;
    }
    
    // 接受客户端连接
    socklen_t client_len = sizeof(ctx->addr);
    ctx->client_sockfd = accept(ctx->sockfd, (struct sockaddr*)&ctx->addr, &client_len);
    if (ctx->client_sockfd < 0) {
        perror("accept");
        close_socket(ctx->sockfd);
        free(ctx);
        free(hw_if);
        return NULL;
    }
    
    printf("[SOCKET] Client connected\n");
    return hw_if;
}

void socket_interface_destroy(HardwareInterface** hw_if) {
    if (!hw_if || !*hw_if) return;
    
    HardwareInterface* iface = *hw_if;
    if (iface->ops.close) {
        iface->ops.close(iface);
    }
    
    free(iface->context);
    free(iface);
    *hw_if = NULL;
}