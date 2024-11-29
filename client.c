#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "192.168.110.5"
#define SERVER_PORT 21003
#define FILE_NAME "send.txt"
#define BUFFER_SIZE 1024

int initialize_socket_connection(const char *ip, int port) {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Error: Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_address.sin_addr) <= 0) {
        perror("Error: Invalid IP address");
        exit(EXIT_FAILURE);
    }

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error: Connection to server failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}

void send_file_data(FILE *file, int socket_fd) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        ssize_t sent_bytes = send(socket_fd, buffer, bytes_read, 0);
        if (sent_bytes == -1) {
            perror("Error: Failed to send file data");
            fclose(file);
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
    }
}

FILE *open_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error: Unable to open file");
        exit(EXIT_FAILURE);
    }
    return file;
}

int main() {
    FILE *file = open_file(FILE_NAME);
    int socket_fd = initialize_socket_connection(SERVER_IP, SERVER_PORT);
    send_file_data(file, socket_fd);
    printf("File '%s' has been successfully sent to the server.\n", FILE_NAME);
    fclose(file);
    close(socket_fd);

    return 0;
}
