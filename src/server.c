/* library implementation */
#include "server.h"

/* standard library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/* system libraries */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


/* buffer definition */
#define BUFFER_SIZE 4096

/* access-gate failures */
#define CLIENT_FD_CONNECTION_FAILURE -1
#define CLIENT_FD_MESSAGE_FAILURE -2


/* 1 ----- set up the access-gate server ----- */
int access_gate_set() {

    /* open socket */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "[ERROR] !!! [ACCESS-GATE CONNECTION] : the socket cannot be opened correctly\n");
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* set socket options */
    int options = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options));

    /* set socket address */
    struct sockaddr_in socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(GATE_PORT);

    /* bind socket-port */
    if (bind(server_fd, (struct sockaddr *)&socket_address, sizeof(socket_address)) < 0) {
        fprintf(stderr, "[ERROR] !!! [ACCESS-GATE CONNECTION] : binding on port %d failed\n", GATE_PORT);
        perror("bind");
        exit(EXIT_FAILURE);
    }

    /* open TCP connection */
    if (listen(server_fd, 16) < 0) {
        fprintf(stderr, "[ERROR] !!! [ACCESS-GATE CONNECTION] : the TCP connection cannot be established correctly\n");
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /* TCP connection opened */
    return server_fd;

}


/* 2 ----- handle a client connection to the access gate ----- */
int access_gate_handle_client(const int server_fd, char *ip_address, char *host) {

    /* accept client connection */
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) return CLIENT_FD_CONNECTION_FAILURE;

    /* read client message  */
    char buffer[BUFFER_SIZE];
    ssize_t bytes_number = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_number < 0) {
        close(client_fd);
        return CLIENT_FD_MESSAGE_FAILURE;
    }
    buffer[bytes_number] = '\0';

    /* parse HTTP lines  */
    char *line = buffer, *end = buffer + bytes_number;
    while (line < end) {
        char *eol = memchr(line, '\r', (size_t)(end - line));
        if (!eol) break;
        size_t line_length = (size_t)(eol - line);

        /* verify header validity */
        size_t header_length, header_subject_length;
        char *header_subject;
        if (line_length > 16 && strncasecmp(line, "x-forwarded-for:", 16) == 0) {
            header_length = 16;
            header_subject = ip_address;
            header_subject_length = IP_ADDRESS_SIZE;
        } else if (line_length > 17 && strncasecmp(line, "x-forwarded-host:", 17) == 0) {
            header_length = 17;
            header_subject = host;
            header_subject_length = HOST_SIZE;
        } else {
            line = eol + 2;
            continue;
        }

        /* get body  */
        char *body = line + header_length;
        while (*body == ' ') body++;
        size_t body_length = (size_t)((line + line_length) - body);
        if (body_length < header_subject_length) {
            memcpy(header_subject, body, body_length);
            header_subject[body_length] = '\0';
        }
        line = eol + 2;
    }

    /* client handled */
    return client_fd;

}