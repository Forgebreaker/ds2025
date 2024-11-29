#define _DEFAULT_SOURCE
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>

void writeFile(int fd, char *filepath)
{
    int file_fd = open(filepath, O_RDONLY);
    if (file_fd < 0)
    {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read;
    char buf[1024];
    while ((bytes_read = read(file_fd, buf, sizeof(buf))) > 0)
    {
        ssize_t bytes_written = write(fd, buf, bytes_read);
        if (bytes_written < 0)
        {
            perror("Error writing to socket");
            close(file_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read < 0)
    {
        perror("Error reading from file");
        close(file_fd);
        exit(EXIT_FAILURE);
    }

    close(file_fd);
}

int main(int argc, char *argv[])
{
    int opt = 1;
    int servfd = socket(AF_INET, SOCK_STREAM, 0);
    if (servfd < 0)
    {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    if (setsockopt(servfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Error setting socket option");
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(servfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Error binding socket");
        return EXIT_FAILURE;
    }

    if (listen(servfd, 3) < 0)
    {
        perror("Socket cannot listen");
        return EXIT_FAILURE;
    };

    int addrlen = sizeof(addr);
    int clientfd = accept(servfd, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
    if (clientfd < 0)
    {
        perror("Error accepting socket");
        return EXIT_FAILURE;
    }

    char filepath[PATH_MAX] = {0};
    ssize_t bytes_read = read(clientfd, filepath, sizeof(filepath) - 1);
    if (bytes_read == 0)
    {
        printf("Client disconnected\n");
    }
    else if (bytes_read < 0)
    {
        perror("Error reading from client");
        close(clientfd);
        close(servfd);
        return EXIT_FAILURE;
    }
    else
    {
        printf("Client require: %s\n", filepath);
        writeFile(clientfd, filepath);
        printf("Write complete, exit!\n");
    }

    close(clientfd);
    close(servfd);
    return EXIT_SUCCESS;
}
