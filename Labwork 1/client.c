#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>

void writeToStream(int fd, char *str)
{
    FILE *stream = fdopen(fd, "w");
    if (stream == NULL)
    {
        perror("Error opening stream");
        exit(EXIT_FAILURE);
    }
    size_t bytesWritten = fwrite(str, 1, strlen(str), stream);
    if (bytesWritten < strlen(str))
    {
        perror("Error writing");
        exit(EXIT_FAILURE);
    }
    fflush(stream);
}

void handleFile(int fd, char *savelocation)
{
    int file_fd;
    char buf[1024];
    // Open file for writing, create if not exists, truncate if exists
    file_fd = open(savelocation, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_fd == -1)
    {
        perror("Error opening file\n");
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buf, sizeof(buf))) > 0)
    {
        ssize_t bytes_written = write(file_fd, buf, bytes_read);
        if (bytes_written < 0)
        {
            perror("Error writing to file\n");
            exit(EXIT_FAILURE);
        }
    }
    if (bytes_read < 0)
    {
        perror("Error reading from socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Received file!\n");
    close(file_fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <filetodownload> <savelocation>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        perror("Error creating socket\n");
        return EXIT_FAILURE;
    }

    char *filedownload = argv[1];
    char *savelocation = argv[2];
    struct sockaddr_in sv_addr;
    sv_addr.sin_family = AF_INET;
    sv_addr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &sv_addr.sin_addr);

    if (connect(clientfd, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) < 0)
    {
        perror("Error connect to socket\n");
        close(clientfd);
        return EXIT_FAILURE;
    }

    writeToStream(clientfd, filedownload);
    handleFile(clientfd, savelocation);
    close(clientfd);
    return EXIT_SUCCESS;
}
