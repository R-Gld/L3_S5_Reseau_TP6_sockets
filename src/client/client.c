#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define handle_error(vl, msg) \
    if(vl == -1) { perror(msg); return EXIT_FAILURE; }
#define handle_error_socket(vl, msg, server_sock_fd) \
    if(vl == -1) { perror(msg); if (close(server_sock_fd) == -1) perror("close socket"); return EXIT_FAILURE; }

/**
 * Params: <pre><code>argv[0] \<address> \<port> [raw]</code></pre>
 */
int main(int argc, char **argv) {
    if(argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <address_str> <port> [raw]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* address_str = argv[1];
    int port = atoi(argv[2]);
    if(port < 1 || port > 65535) { fprintf(stderr, "The port should be an int between 1 and 65535.\n"); return EXIT_FAILURE; }

    int raw = 0;
    if(argc == 4 && strcmp(argv[3], "true") == 0) {
        raw = 1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0; hints.ai_protocol = 0;

    struct addrinfo *result, *rp;

    int s = getaddrinfo(address_str, argv[2], &hints, &result);
    if(s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    int socket_fd;
    for(rp = result; rp != NULL; rp = rp->ai_next) {
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(socket_fd == -1) continue;
        if(connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        close(socket_fd);
    }
    freeaddrinfo(result);
    if(rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }



    /*struct sockaddr_in sa;
    sa.sin_port = htons(port);
    sa.sin_family = AF_INET;
    bzero(sa.sin_zero, 8);

    int err = inet_aton(address_str, &sa.sin_addr);
    if(err == 0) { fprintf(stderr, "An error occurred while parsing the IP '%s'.\n", address_str); return EXIT_FAILURE; }


    int socket_fd = socket(sa.sin_family, SOCK_STREAM, 0);
    handle_error(socket_fd, "socket");

    err = connect(socket_fd, (struct sockaddr*) &sa, sizeof(struct sockaddr));
    handle_error_socket(err, "connect", socket_fd)*/

    char buffer[BUFSIZ]; // Match the size from the BUFFER to the server.c
    ssize_t byte_read = 1;
    while(byte_read >= 0) {
        byte_read = recv(socket_fd, buffer, BUFSIZ, 0);
        if(byte_read == 0) {
            if(!raw) printf("Closing connection.\n");
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
            if(raw) {
                printf("%s", buffer);
            } else {
                printf("Message reçu:\n```\n%s\n```\n", buffer);
            }
        }
    }

    return EXIT_SUCCESS;
}