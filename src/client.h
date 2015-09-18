#ifndef CLIENT_H_
#define CLIENT_H_

#include "parser.h"

typedef struct {
    pthread_t thread_id; // reserved for further use
    int msgsock;          // Socket for the connection - valid if > 0
    char ipstr[INET6_ADDRSTRLEN];
    http_header_t *header;
} Client;

// get client connection information
void get_peer_information(Client *client);
char *read_file(char *path, size_t *file_size);
int read_http_request(Client *client);
int make_http_response(Client *client);


#endif
