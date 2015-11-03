#ifndef RESPONSE_H_
#define RESPONSE_H_


#include "config.h"
#include "client.h"

int http_response_get(Configuration *config, Client *client);
int http_response_post(Configuration *config, Client *client);
int http_response_default(Configuration *config, Client *client);

// send http response to client
int make_http_response(Configuration *config, Client *client);

#endif
