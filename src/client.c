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
#include "http_parser.h"
#include "response.h"


// implementation for strnstr which is not available on linux
// source from https://github.com/torvalds/linux/blob/master/lib/string.c
char *strnstr(const char *s1, const char *s2, size_t len)
{
    size_t l2;

    l2 = strlen(s2);
    if (!l2)
    return (char *)s1;
    while (len >= l2) {
        len--;
        if (!memcmp(s1, s2, l2))
        return (char *)s1;
        s1++;
    }
    return NULL;
}


void get_peer_information(Client *client)
{
    socklen_t len;
    struct sockaddr_storage addr;
    char ipstr[INET6_ADDRSTRLEN];

    len = sizeof(addr);
    getpeername(client->msgsock, (struct sockaddr*)&addr, &len);

    // deal with both IPv4 and IPv6:
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    } else { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }

    strcpy(client->ipstr, ipstr);
}


void handle_http_request(Configuration *config, Client *client)
{
    int                 rc;
    ssize_t 			sz;
    size_t 				readed;
    char 				buffer[READ_SZ];
    char 				*data;
    char 				*header_end;
    ssize_t             content_length;

    // init variables
    content_length = -1;
    readed = 0;
    data = NULL;
    header_end = NULL;
    memset(buffer, 0, READ_SZ);

    while ((sz = read(client->msgsock, buffer, READ_SZ)) != 0)
    {
        if (sz < 0)
        {
            handle_error("read");
        }
        else
        {
            // try to find termination
            header_end = strnstr(buffer, "\r\n\r\n", sz);

            // if found, then append text before termination, process header
            // if not found, append to data
            if (header_end)
            {
                sz = header_end - buffer + 4;
            }

            // use realloc to increase size for variable length data
            data = realloc(data, readed + sz + 1);

            // copy downloaded data from buffer to data
            memcpy(data + readed, buffer, sz);

            // increase downloaded size
            readed += sz;

            // process header
            if (header_end)
            {
                printf("\n----------\nRECV [%s]:\n%s\n----------\n", client->ipstr, data);
                // get http header
                http_header_t *header = (http_header_t *) malloc (sizeof(http_header_t));

                // parse http header
                rc = parse(header, data, sz);
                client->header = header;

                // extract payload
                for (int i = 0; i < client->header->num_fields; i++)
                {
                    if (strcasecmp(client->header->fields[i], "Content-Length") == 0)
                    {
                        content_length = atoi(client->header->values[i]);
                        break;
                    }
                }
                if (content_length > 0)
                {
                    if (READ_SZ - sz >= content_length)
                    {
                        client->payload = malloc(content_length + 1);
                        memcpy(client->payload, buffer + sz, content_length);
                    }
                    else
                    {
                        int remained;
                        int copied;

                        copied = READ_SZ - sz;
                        remained = content_length - copied;

                        client->payload = malloc(content_length + 1);
                        memcpy(client->payload, buffer + sz, copied);

                        char payload_buffer[remained + 1];

                        // receive remained payload from socket
                        sz = read(client->msgsock, payload_buffer, content_length);
                        memcpy(client->payload + copied, payload_buffer, remained);
                    }
                }
                else
                {
                    client->payload = NULL;
                }

                // clean up received header
                free(data);

                // make http response
                make_http_response(config, client);

                // clean up http response
                free(client->header);
                if (client->payload)
                {
                    free(client->payload);
                }
                free(client);


                break;
            }

        }

    }
}
