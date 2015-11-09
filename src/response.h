#ifndef RESPONSE_H_
#define RESPONSE_H_


#include "config.h"
#include "client.h"


// send http response to client
int make_http_response(Configuration *config, Client *client);

#endif
