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

char *read_file_stdio(char *path, size_t *file_size)
{
    FILE *fp = NULL;


    // open file
    fp = fopen(path, "r");
    if (fp == NULL)
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
    while (fgets(strbuf, READ_SZ, fp) != NULL)
    {

        // append
        sz = strlen(strbuf);
        file_buffer = realloc(file_buffer, *file_size + sz + 1);
        memcpy(file_buffer + *file_size, strbuf, sz);
        *file_size += sz;
        file_buffer[*file_size] = 0;


        memset(strbuf, 0, READ_SZ);
    }
    fclose(fp);

    return file_buffer;
}


int http_response_get(Configuration *config, Client *client)
{
    int rc;
    int http_code = 0;
    char datetime[MAX_DATETIME_LENGTH];
    char path[MAX_PATH_SZ];
    char default_dir[MAX_VALUE_LEN];
    char value[MAX_VALUE_LEN];
    char *file_buffer = NULL;
    size_t file_size;

    // get datetime
    memset(datetime, 0, MAX_DATETIME_LENGTH);
    rc = get_datetime(datetime);

    // get default dir
    memset(value, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "default_dir", default_dir);

    // get html path
    sprintf(path, "%s%s", default_dir, client->header->url);

    printf("HTTP_PATH: %s\n", path);

    // try to read file
    file_buffer = read_file(path, &file_size);

    http_code = 200;

    // page not found
    if (file_buffer == NULL)
    {
        if (errno == EISDIR)
        {
            memset(value, 0, MAX_VALUE_LEN);
            rc = config_get_value(config, "default_index_page", value);
            sprintf(path, "%s%s%s", default_dir, client->header->url, value);
            printf("HTTP_PATH_FIX: %s\n", path);

            file_buffer = read_file(path, &file_size);
        }
        else
        {
            http_code = 404;
            printf("HTTP_CODE: %d\n", http_code);

            memset(value, 0, MAX_VALUE_LEN);
            rc = config_get_value(config, "default_404_page", value);
            if (!rc)
            {
                write(client->msgsock, HTTP_RESPONSE_404, strlen(HTTP_RESPONSE_404));
                printf("TRY1\n");
                return http_code;
            }
            sprintf(path, "%s%s", default_dir, value);

            file_buffer = read_file(path, &file_size);

            // NO! We should have 404 page sit somewhere
            if (file_buffer == NULL)
            {
                write(client->msgsock, HTTP_RESPONSE_404, strlen(HTTP_RESPONSE_404));
                printf("TRY2\n");
                return http_code;
            }
        }
    }

    // response http_packet
    char buf[file_size + MAX_DATETIME_LENGTH + strlen(HTTP_RESPONSE_TEMPLATE) * 2];
    size_t readable_length;

    memset(buf, 0, sizeof(buf));

    // create http header
    sprintf(buf,
        HTTP_RESPONSE_TEMPLATE, // template
        http_code,
        get_http_code_description(http_code),
        file_size,
        datetime);

    strncat(buf, "\r\n", 2);

    readable_length = strlen(buf);

    // append binary data (possible) to the end of header
    memcpy(buf + readable_length, file_buffer, file_size);

    // append \r\n at the end
    memcpy(buf + readable_length + file_size, "\r\n", 3);

    write(client->msgsock, buf, sizeof(buf));


    // safely free file buffer
    if (file_buffer != NULL)
    {
        free(file_buffer);
    }

    return http_code;
}


int http_response_post(Configuration *config, Client *client)
{
    int rc;
    int http_code = 0;
    char datetime[MAX_DATETIME_LENGTH];
    char path[MAX_PATH_SZ];
    char default_dir[MAX_VALUE_LEN];
    char value[MAX_VALUE_LEN];
    char *output_buffer = NULL;
    size_t output_size;

    // get datetime
    memset(datetime, 0, MAX_DATETIME_LENGTH);
    rc = get_datetime(datetime);

    // get default dir
    memset(value, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "default_dir", default_dir);

    // get cgi path
    sprintf(path, "%s%s", default_dir, client->header->url);
    printf("CGI_PATH: %s\n", path);

    // execute cgi script
    output_buffer = execute_cgi(path, &output_size, client);

    http_code = 200;

    // failed to execute
    if (output_buffer == NULL)
    {
        if (errno == EISDIR)
        {
            memset(value, 0, MAX_VALUE_LEN);
            rc = config_get_value(config, "default_index_page", value);
            sprintf(path, "%s%s%s", default_dir, client->header->url, value);
            printf("CGI_PATH_FIX: %s\n", path);

            output_buffer = execute_cgi(path, &output_size, client);
        }
        else
        {
            http_code = 500;
            printf("HTTP_CODE: %d\n", http_code);

            memset(value, 0, MAX_VALUE_LEN);
            rc = config_get_value(config, "default_500_page", value);
            if (!rc)
            {
                write(client->msgsock, HTTP_RESPONSE_500, strlen(HTTP_RESPONSE_500));
                printf("TRY1\n");
                return http_code;
            }
            sprintf(path, "%s%s", default_dir, value);

            output_buffer = read_file(path, &output_size);

            // NO! We should have 500 page sit somewhere
            if (output_buffer == NULL)
            {
                write(client->msgsock, HTTP_RESPONSE_500, strlen(HTTP_RESPONSE_500));
                printf("TRY2\n");
                return http_code;
            }
        }
    }



    // response http_packet
    char buf[output_size + MAX_DATETIME_LENGTH + strlen(HTTP_RESPONSE_TEMPLATE) * 2];
    size_t readable_length;

    memset(buf, 0, sizeof(buf));

    // create http header
    sprintf(buf,
        HTTP_RESPONSE_TEMPLATE, // template
        http_code,
        get_http_code_description(http_code),
        output_size,
        datetime);

    // header may not yet complete so we dont need to terminate header
    //strncat(buf, "\r\n", 2);

    readable_length = strlen(buf);

    // append binary data (possible) to the end of header
    memcpy(buf + readable_length, output_buffer, output_size);

    // append \r\n at the end
    memcpy(buf + readable_length + output_size, "\r\n", 3);

    write(client->msgsock, buf, sizeof(buf));


    // safely free file buffer
    if (output_buffer != NULL)
    {
        free(output_buffer);
    }

    return http_code;
}


int http_response_default(Configuration *config, Client *client)
{
    int http_code = 0;

    http_code = 501;
    write(client->msgsock, HTTP_RESPONSE_501, strlen(HTTP_RESPONSE_501));
    printf("501 Method Not Implemented\n");
    return http_code;
}


int make_http_response(Configuration *config, Client *client)
{
    // serve file
    switch (client->header->method)
    {

        case 1: // 1 is GET
        {
            http_response_get(config, client);
            break;
        }

        case 3: // 3 is POST
        {
            http_response_post(config, client);
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
