#ifndef CGI_H_
#define CGI_H_


#include "client.h"

#define MAX_QUERY_SZ 1024

int execute_python(char *path, Configuration *config, Client *client);

#endif
