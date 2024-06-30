#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8888
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024

int main() {
    int sockfd, client_sockfd, bytes_received;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char *response = "Hello, client!";
    
    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 配置服务器端地址、端口信息
    bzero(&serv_addr, sizeof(serv_addr));//清空内容
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // 绑定地址结构体到套接字上
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 设置套接字为监听状态
    if (listen(sockfd, MAX_CONNECTIONS) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 接收新的客户端请求
    client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
    if (client_sockfd == -1) {
        perror("accept");
    }
    printf("client connected \r\n");

    while (1) {

        // 接收客户端数据
        bytes_received = recv(client_sockfd, buffer, BUFFER_SIZE, 0);
        if (bytes_received == -1) {
            perror("recv");
            close(client_sockfd);
            continue;
        }
        if(bytes_received > 0)
            printf("Received %d bytes from client: %s\n", bytes_received, buffer);

        // // 向客户端发送数据
        // if (send(client_sockfd, response, strlen(response), 0) == -1) {
        //     perror("send");
        //     close(client_sockfd);
        //     continue;
        // }

        // printf("Sent response to client: %s\n", response);

        // 关闭当前套接字
        // close(client_sockfd);
        // printf("Client connection closed\n");
    }

    // 关闭服务端套接字
    close(sockfd);

    return 0;
}



// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>

// #define PORT 8888
// #define MAX_CONNECTIONS 5
// #define BUFFER_SIZE 1024

// int create_server_socket() {
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd == -1) {
//         perror("socket");
//         exit(EXIT_FAILURE);
//     }
//     return sockfd;
// }

// void initialize_server_address(struct sockaddr_in *serv_addr) {
//     bzero(serv_addr, sizeof(*serv_addr));
//     serv_addr->sin_family = AF_INET;
//     serv_addr->sin_addr.s_addr = INADDR_ANY;
//     serv_addr->sin_port = htons(PORT);
// }

// void bind_socket(int sockfd, struct sockaddr_in serv_addr) {
//     if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
//         perror("bind");
//         exit(EXIT_FAILURE);
//     }
// }

// void start_listening(int sockfd, int max_connections) {
//     if (listen(sockfd, max_connections) == -1) {
//         perror("listen");
//         exit(EXIT_FAILURE);
//     }
//     printf("Server listening on port %d...\n", PORT);
// }

// int accept_connection(int sockfd, struct sockaddr_in *client_addr) {
//     socklen_t client_addrlen = sizeof(*client_addr);
//     int client_sockfd = accept(sockfd, (struct sockaddr *)client_addr, &client_addrlen);
//     if (client_sockfd == -1) {
//         perror("accept");
//     }
//     return client_sockfd;
// }

// void handle_client(int client_sockfd) {
//     char buffer[BUFFER_SIZE];
//     char *response = "Hello, client!";
//     int bytes_received = recv(client_sockfd, buffer, BUFFER_SIZE, 0);
//     if (bytes_received == -1) {
//         perror("recv");
//     } else {
//         printf("Received %d bytes from client: %s\n", bytes_received, buffer);
//         if (send(client_sockfd, response, strlen(response), 0) == -1) {
//             perror("send");
//         } else {
//             printf("Sent response to client: %s\n", response);
//         }
//     }
//     close(client_sockfd);
//     printf("Client connection closed\n");
// }

// void close_socket(int sockfd) {
//     close(sockfd);
// }

// int main() {
//     int sockfd, client_sockfd;
//     struct sockaddr_in serv_addr, client_addr;

//     sockfd = create_server_socket();
//     initialize_server_address(&serv_addr);
//     bind_socket(sockfd, serv_addr);
//     start_listening(sockfd, MAX_CONNECTIONS);

//     while (1) {
//         client_sockfd = accept_connection(sockfd, &client_addr);
//         if (client_sockfd >= 0) {
//             handle_client(client_sockfd);
//         }
//     }

//     close_socket(sockfd);
//     return 0;
// }
