#include "socket.h"

int create_socket(void) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
    }
    return sockfd;
}

int socket_write(int sockfd, uint8_t *data, uint16_t length) {
    int ret = send(sockfd, data, length, 0);
    if (ret == -1) {
        perror("send");
        close(sockfd);
    }
    return ret;
}

int socket_read(int sockfd, char *data, uint16_t length) {
    int ret = recv(sockfd, data, length, 0);
    if (ret == -1) {
        perror("recv");
        close(sockfd);
    }
    return ret;
}

void close_socket(int sockfd) {
    close(sockfd);
}

/************************client*******************************/

int socket_client_address_init(struct sockaddr_in *serv_addr, const char* ip_addr, uint16_t port) {
    bzero(serv_addr, sizeof(*serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(port);
    if (inet_pton(AF_INET, ip_addr, &serv_addr->sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    return 0;
}

int connect_to_server(int sockfd, struct sockaddr_in* serv_addr) {
    if (connect(sockfd, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in)) == -1) {
        perror("connect");
        close(sockfd);
        return -1;
    }
    printf("Connected to server: %s:%d\n", inet_ntoa(serv_addr->sin_addr), ntohs(serv_addr->sin_port));
    return 0;
}

/************************server*******************************/

void socket_server_address_init(struct sockaddr_in *serv_addr, uint16_t port) 
{
    bzero(serv_addr, sizeof(*serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = INADDR_ANY;
    serv_addr->sin_port = htons(port);
}

int socket_server_bind_socket(int sockfd, struct sockaddr_in* serv_addr) 
{
    if (bind(sockfd, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) == -1) {
        perror("bind");
        return -1;
    }
    return 0;
}

int socket_server_start_listening(int sockfd, int max_connections) 
{
    if (listen(sockfd, max_connections) == -1) {
        perror("listen");
        return -1;
    }
    return 0;
}

int socket_server_accept_connection(int sockfd, struct sockaddr_in *client_addr) 
{
    socklen_t client_addrlen = sizeof(*client_addr);
    int client_sockfd = accept(sockfd, (struct sockaddr *)client_addr, &client_addrlen);
    if (client_sockfd == -1) {
        perror("accept");
        return -1;
    }
    return client_sockfd;
}

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

