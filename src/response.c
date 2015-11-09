#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#include "client.h"
#include "datetime.h"
#include "parser.h"
#include "config.h"
#include "response.h"
#include "http_parser.h"
#include "cgi.h"


int make_http_response(Configuration *config, Client *client)
{
    int                     rc;
    char                    default_dir[MAX_VALUE_LEN];
    char                    value[MAX_VALUE_LEN];
    int                     error_code;

    // execute python script
    error_code = execute_python(client->header->url, config, client);

    if (error_code)
    {
        if (error_code == ENOENT)
        {
            memset(value, 0, MAX_VALUE_LEN);
            rc = config_get_value(config, "default_404_page", value);

            error_code = execute_python(value, config, client);
        }
        else
        {
            memset(value, 0, MAX_VALUE_LEN);
            rc = config_get_value(config, "default_500_page", value);

            error_code = execute_python(value, config, client);
        }
    }

    return error_code;

    return 0;
}
