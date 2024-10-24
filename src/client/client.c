#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>


/**
 * Params: argv[0] <address> <port>
 */
int main(int argc, char **argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: %s <address_str> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* address_str = argv[1];
    int port = atoi(argv[2]);
    if(port < 1 || port > 65535) { fprintf(stderr, "The port should be an int between 1 and 65535.\n"); return EXIT_FAILURE; }

    struct sockaddr_in sa;
    sa.sin_port = htons(port);
    sa.sin_family = AF_INET;
    bzero(sa.sin_zero, 8);

    int err = inet_aton(address_str, &sa.sin_addr);
    if(err == 0) { fprintf(stderr, "Error while trying to parse the address '%s'\n", address_str); return EXIT_FAILURE; }


    int socket_fd = socket(sa.sin_family, SOCK_STREAM, 0);
    if(socket_fd == -1) { perror("socket"); return EXIT_FAILURE; }

    err = connect(socket_fd, (struct sockaddr*) &sa, sizeof(struct sockaddr));
    if(err == -1) {
        perror("connect");
        if(close(socket_fd) == -1) perror("close socket");
        return EXIT_FAILURE;
    }

    char buffer[BUFSIZ];
    ssize_t byte_read = 1;
    while(byte_read >= 0) {
        byte_read = recv(socket_fd, buffer, BUFSIZ, 0);
        if(byte_read == 0) {
            printf("Closing connection.\n");
            if(shutdown(socket_fd, SHUT_RDWR) == -1) { perror("shutdown"); return EXIT_FAILURE; }
            if(close(socket_fd) == -1) perror("close socket");
            break;
        } else if(byte_read == -1) {
            perror("recv");
            if(shutdown(socket_fd, SHUT_RDWR) == -1) { perror("shutdown"); return EXIT_FAILURE; }
            if(close(socket_fd) == -1) perror("close socket");
            return EXIT_FAILURE;
        } else {
            buffer[byte_read] = '\0';
            printf("Message reçu:\n```\n%s\n```\n", buffer);
        }
    }

    return EXIT_SUCCESS;
}