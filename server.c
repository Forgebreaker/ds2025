#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define IP_ADDRESS "192.168.110.5"
#define PORT 21003
#define FILE_PATH "first.txt"
#define BUF_SIZE 1024

int create_client_socket() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    return sock_fd;
}

void configure_server_address(struct sockaddr_in *server_addr) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, IP_ADDRESS, &server_addr->sin_addr) <= 0) {
        perror("Invalid IP address");
        exit(EXIT_FAILURE);
    }
}

void connect_to_server(int sock_fd, struct sockaddr_in *server_addr) {
    if (connect(sock_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0) {
        perror("Server connection failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

FILE *open_file_for_reading(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("File opening error");
        exit(EXIT_FAILURE);
    }
    return file;
}

void transfer_file_data(FILE *file, int sock_fd) {
    char buffer[BUF_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUF_SIZE, file)) > 0) {
        if (send(sock_fd, buffer, bytes_read, 0) < 0) {
            perror("Error sending data");
            fclose(file);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }
}

void close_resources(FILE *file, int sock_fd) {
    fclose(file);
    close(sock_fd);
}

int main() {
    FILE *file = open_file_for_reading(FILE_PATH);
    int sock_fd = create_client_socket();
    
    struct sockaddr_in server_addr;
    configure_server_address(&server_addr);
    
    connect_to_server(sock_fd, &server_addr);
    transfer_file_data(file, sock_fd);
    
    printf("Successfully sent '%s' to the server.\n", FILE_PATH);
    
    close_resources(file, sock_fd);
    return 0;
}
