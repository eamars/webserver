#ifndef CGI_H_
#define CGI_H_


#include "client.h"


char *execute_cgi(char *path, size_t *output_size, Client *client);

#endif
