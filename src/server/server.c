#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LISTENING_ADDRESS "0.0.0.0"

#define MESSAGE_SENT "Const message sent from the server. :)\n"  // Message sent to the client

/**
 * Params: argv[0] <port>
 */
int main(int argc, char **argv) {
    if(strlen(MESSAGE_SENT) + 1 > BUFSIZ) { // Adding +1 to strlen to include the '\0' char.
        fprintf(stderr, "The message is too large to be sent entirely to the client. The maximum size is %d.\n", BUFSIZ);
        return EXIT_FAILURE;
    }

    if(argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int port = atoi(argv[1]);
    if(port < 1 || port > 65535) { fprintf(stderr, "Port should be an int between 1 and 65535.\n"); return EXIT_FAILURE; }

    struct sockaddr_in sa;
    sa.sin_port = htons(port);
    sa.sin_family = AF_INET;
    bzero(sa.sin_zero, 8);

    int err = inet_aton(LISTENING_ADDRESS, &sa.sin_addr); // convert string version of the listening address to a binary form (in network byte order)
    if(err == 0) { fprintf(stderr, "An error occurred while parsing the IP '%s'.\n", LISTENING_ADDRESS); return EXIT_FAILURE; }

    int server_sock_fd = socket(sa.sin_family, SOCK_STREAM, 0);
    if(server_sock_fd == -1) { perror("socket"); return EXIT_FAILURE; }

    struct sockaddr_in peer_addr;

    err = bind(server_sock_fd, (struct sockaddr *) &sa, sizeof(struct sockaddr));
    if(err == -1) { perror("bind"); if (close(server_sock_fd) == -1) perror("close socket"); return EXIT_FAILURE; }

    err = listen(server_sock_fd, SOMAXCONN); // Here, `SOMAXCONN` as backlog to use `/proc/sys/net/core/somaxconn` (The system defined backlog value) By default on my system: 4096
    if(err == -1) { perror("listen"); if (close(server_sock_fd) == -1) perror("close socket"); return EXIT_FAILURE; }

    socklen_t peer_addr_size = sizeof(peer_addr);
    int client_socket_fd = accept(server_sock_fd, (struct sockaddr *) &peer_addr, &peer_addr_size);
    if(client_socket_fd == -1) { perror("accept"); if(close(server_sock_fd) == -1) perror("close socket"); return EXIT_FAILURE; }

    const char *message = MESSAGE_SENT;

    /**
     * Send the message to the client
     * Return the number of byte sent on success and -1 on error (errno set)
     */
    ssize_t byte_sent = send(client_socket_fd, message, strlen(message), 0);
    if(byte_sent == -1) {
        perror("send");
        if (shutdown(client_socket_fd, SHUT_RDWR) == -1) perror("shutdown client socket ends");
        if (close(client_socket_fd) == -1) perror("close client socket");

        if(shutdown(server_sock_fd, SHUT_RDWR) == -1) perror("shutdown server socket ends");
        if(close(server_sock_fd) == -1) perror("close server socket");

        return EXIT_FAILURE;
    }

    // Close every socket.
    // Before using close(2), using shutdown(2) to shutdown gracefully the sockets.
    if(shutdown(client_socket_fd, SHUT_RDWR) == -1) { perror("shutdown client socket ends"); return EXIT_FAILURE; }
    if(close(client_socket_fd) == -1) { perror("close client socket"); return EXIT_FAILURE; }

    if(shutdown(server_sock_fd, SHUT_RDWR) == -1) { perror("shutdown server socket ends"); return EXIT_FAILURE; }
    if(close(server_sock_fd) == -1) { perror("close server socket"); return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}