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


const char *HTTP_RESPONSE_TEMPLATE =
"HTTP/1.1 %d %s\r\n"
"Server: webhttpd/1.0\r\n"
"Cache-Control: no-cache, no-store, must-revalidate\r\n"
"Connection: close\r\n"
"Content-Length %ld\r\n"
"Date: %s\r\n";

const char *HTTP_RESPONSE_404 =
"HTTP/1.1 404 Not Found\r\n"
"Content-Type: text/html\r\n"
"\r\n"
"<HTML><TITLE>404</TITLE>\r\n"
"<BODY><P>404 - Page Not Found</P></BODY>\r\n"
"</HTML>\r\n"
"\r\n";

const char *HTTP_RESPONSE_501 =
"HTTP/1.1 501 Method Not Implemented\r\n"
"Content-Type: text/html\r\n"
"\r\n"
"<HTML><TITLE>501</TITLE>\r\n"
"<BODY><P>501 - Method Not Implemented</P></BODY>\r\n"
"</HTML>\r\n"
"\r\n";

const char *HTTP_RESPONSE_500 =
"HTTP/1.1 500 Internal Server Error\r\n"
"Content-Type: text/html\r\n"
"\r\n"
"<HTML><TITLE>500</TITLE>\r\n"
"<BODY><P>500 - Internal Server Error</P></BODY>\r\n"
"</HTML>\r\n"
"\r\n";


char *read_file(char *path, size_t *file_size)
{
    int fd;


    // open file
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return NULL;
    }

    // read file
    int sz;
    char *file_buffer = NULL;
    char strbuf[READ_SZ];
    *file_size = 0;

    memset(strbuf, 0, READ_SZ);
    while ((sz = read(fd, strbuf, READ_SZ)) != 0)
    {
        if (sz < 0)
        {
            perror("read");
            return NULL;
        }

        // append
        file_buffer = realloc(file_buffer, *file_size + sz);
        memcpy(file_buffer + *file_size, strbuf, sz);
        *file_size += sz;

        memset(strbuf, 0, READ_SZ);
    }
    close(fd);

    return file_buffer;
}


int http_response_get_method(Configuration *config, Client *client)
{
    int                     rc;
    char                    default_dir[MAX_VALUE_LEN];
    char                    value[MAX_VALUE_LEN];
    char                    path[MAX_PATH_SZ];
    int                     error_code;

    // get default dir
    memset(default_dir, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "default_dir", default_dir);

    // get executable path
    sprintf(path, "%s%s", default_dir, client->header->url);

    error_code = execute_python(path, config, client);

    if (error_code)
    {
        if (error_code == ENOENT)
        {
            memset(value, 0, MAX_VALUE_LEN);
            rc = config_get_value(config, "default_404_page", value);

            sprintf(path, "%s%s", default_dir, value);
            error_code = execute_python(path, config, client);
        }
        else if (error_code == EACCES)
        {
            memset(value, 0, MAX_VALUE_LEN);
            rc = config_get_value(config, "default_500_page", value);

            sprintf(path, "%s%s", default_dir, value);
            error_code = execute_python(path, config, client);
        }
        else
        {
            memset(value, 0, MAX_VALUE_LEN);
            rc = config_get_value(config, "default_501_page", value);

            sprintf(path, "%s%s", default_dir, value);
            error_code = execute_python(path, config, client);
        }
    }

    return error_code;
}

int http_response_default(Configuration *config, Client *client)
{
    int                     rc;
    char                    default_dir[MAX_VALUE_LEN];
    char                    value[MAX_VALUE_LEN];
    char                    path[MAX_PATH_SZ];
    int                     error_code;

    // get default dir
    memset(default_dir, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "default_dir", default_dir);

    // get executable path
    memset(value, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "default_501_page", value);

    sprintf(path, "%s%s", default_dir, value);
    error_code = execute_python(path, config, client);

    return error_code;
}


int make_http_response(Configuration *config, Client *client)
{
    // serve file
    switch (client->header->method)
    {

        case 1: // 1 is GET
        {
            http_response_get_method(config, client);
            break;
        }

        case 3: // 3 is POST
        {
            // TODO: implement http post
            break;
        }

        default: // unimplemented
        {
            http_response_default(config, client);
            break;
        }

    }

    return 0;
}
