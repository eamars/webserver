#ifndef CLIENT_H_
#define CLIENT_H_

#include "parser.h"
#include "config.h"


#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)


#define MAX_QUERY_SZ 255
#define READ_SZ 4096
#define MAX_PATH_SZ 8192


typedef struct {
    pthread_t thread_id; // reserved for further use
    int msgsock;          // Socket for the connection - valid if > 0
    char ipstr[INET6_ADDRSTRLEN];
    http_header_t *header;
    char *payload;
} Client;

// get client connection information
void get_peer_information(Client *client);

// read http request from socket
void handle_http_request(Configuration *config, Client *client);


#endif
