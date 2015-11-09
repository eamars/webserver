#ifndef CLIENT_H_
#define CLIENT_H_

#include <netinet/in.h>

#include "parser.h"
#include "config.h"
#include "error.h"


#define READ_SZ 1024
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

void handle_request(Configuration *config, int msgsock);


#endif
