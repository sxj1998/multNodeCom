#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int sockfd, bytes_sent, bytes_received;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char *request = "Hello, server!";

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 配置地址结构体信息、端口号、IP地址等
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    // 向服务端发送请求连接
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server: %s:%d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));


    // 向服务端发送请求、数据
   bytes_sent = send(sockfd, request, strlen(request), 0);
    if (bytes_sent == -1) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Sent %d bytes to server: %s\n", bytes_sent, request);

    // 接收服务端数据
    bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received == -1) {
        perror("recv");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Received %d bytes from server: %s\n", bytes_received, buffer);

    // Close the socket
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
// #include <arpa/inet.h>

// #define PORT 8888
// #define BUFFER_SIZE 1024

// int create_socket() {
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd == -1) {
//         perror("socket");
//         exit(EXIT_FAILURE);
//     }
//     return sockfd;
// }

// void initialize_address(struct sockaddr_in *serv_addr) {
//     bzero(serv_addr, sizeof(*serv_addr));
//     serv_addr->sin_family = AF_INET;
//     serv_addr->sin_port = htons(PORT);
//     if (inet_pton(AF_INET, "127.0.0.1", &serv_addr->sin_addr) <= 0) {
//         printf("\nInvalid address/ Address not supported \n");
//         exit(EXIT_FAILURE);
//     }
// }

// void connect_to_server(int sockfd, struct sockaddr_in serv_addr) {
//     if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
//         perror("connect");
//         close(sockfd);
//         exit(EXIT_FAILURE);
//     }
//     printf("Connected to server: %s:%d\n", inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));
// }

// void send_request(int sockfd, const char *request) {
//     int bytes_sent = send(sockfd, request, strlen(request), 0);
//     if (bytes_sent == -1) {
//         perror("send");
//         close(sockfd);
//         exit(EXIT_FAILURE);
//     }
//     printf("Sent %d bytes to server: %s\n", bytes_sent, request);
// }

// void receive_response(int sockfd, char *buffer, size_t bufsize) {
//     int bytes_received = recv(sockfd, buffer, bufsize, 0);
//     if (bytes_received == -1) {
//         perror("recv");
//         close(sockfd);
//         exit(EXIT_FAILURE);
//     }
//     buffer[bytes_received] = '\0'; // Null-terminate the received data
//     printf("Received %d bytes from server: %s\n", bytes_received, buffer);
// }

// void close_connection(int sockfd) {
//     close(sockfd);
// }

// int main() {
//     int sockfd;
//     struct sockaddr_in serv_addr;
//     char buffer[BUFFER_SIZE];
//     char *request = "Hello, server!";

//     sockfd = create_socket();
//     initialize_address(&serv_addr);
//     connect_to_server(sockfd, serv_addr);
//     send_request(sockfd, request);
//     receive_response(sockfd, buffer, BUFFER_SIZE);
//     close_connection(sockfd);

//     return 0;
// }




