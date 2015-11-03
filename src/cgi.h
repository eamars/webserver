#ifndef CGI_H_
#define CGI_H_


#include "client.h"

char *execute_python(char *path, size_t *output_size, Configuration *config, Client *client);

#endif
