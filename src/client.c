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

#include "client.h"
#include "datetime.h"
#include "parser.h"

#define MAX_SZ 8192

const char *HTTP_RESPONSE_TEMPLATE =
"HTTP/1.1 %d %s\r\n"
"Server: webhttpd/1.0\r\n"
"Connection: close\r\n"
"Content-Length %ld\r\n"
"Date: %s\r\n"
"\r\n";

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
	char strbuf[4096];
	*file_size = 0;

	memset(strbuf, 0, 4096);
	while ((sz = read(fd, strbuf, 4096)) != 0)
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

		memset(strbuf, 0, 4096);
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
	char strbuf[4096];
	*file_size = 0;

	memset(strbuf, 0, 4096);
	while (fgets(strbuf, 4096, fp) != NULL)
	{

		// append
        sz = strlen(strbuf);
		file_buffer = realloc(file_buffer, *file_size + sz + 1);
		memcpy(file_buffer + *file_size, strbuf, sz);
		*file_size += sz;
        file_buffer[*file_size] = 0;


		memset(strbuf, 0, 4096);
	}
	fclose(fp);

	return file_buffer;
}

int read_http_request(Client *client)
{
	int sz;
	int rc;
	char buffer[MAX_SZ];


	memset(buffer, 0, MAX_SZ);

	sz = read(client->msgsock, buffer, MAX_SZ);
	printf("IP: %s\n", client->ipstr);
	printf("RECV:\n----------\n%s\n----------\n", buffer);

	// get http header
	http_header_t *header = (http_header_t *) malloc (sizeof(http_header_t));

	// parse http header
	rc = parse(header, buffer, sz);

	client->header = header;

	return sz;
}


int http_response_get(Client *client)
{
    int rc;
    int http_code = 0;
    char datetime[MAX_DATETIME_LENGTH];
    char path[MAX_SZ];
    char *file_buffer = NULL;
    size_t file_size;

    // get datetime
	memset(datetime, 0, MAX_DATETIME_LENGTH);
	rc = get_datetime(datetime);

    // get html path
    sprintf(path, "./html%s", client->header->url);
    printf("HTTP_PATH: %s\n", path);

    // try to read file
    file_buffer = read_file(path, &file_size);

    // page not found
    if (file_buffer == NULL)
    {
        http_code = 404;
        file_buffer = read_file("./html/404.html", &file_size);

        // NO! We should have 404 page sit somewhere
        if (file_buffer == NULL)
        {
            write(client->msgsock, HTTP_RESPONSE_404, strlen(HTTP_RESPONSE_404));
            printf("HTTP_CODE: 501\n");
            return http_code;
        }

    }

    http_code = 200;

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

char *execute_cgi(char *path, size_t *output_size)
{
    return NULL;
}
int http_response_post(Client *client)
{
    int rc;
    int http_code = 0;
    char datetime[MAX_DATETIME_LENGTH];
    char path[MAX_SZ];
    char *output_buffer = NULL;
    size_t output_size;

    // get datetime
	memset(datetime, 0, MAX_DATETIME_LENGTH);
	rc = get_datetime(datetime);

    // get cgi path
    sprintf(path, "./html%s", client->header->url);
    printf("CGI_PATH: %s\n", path);

    // execute cgi script
    output_buffer = execute_cgi(path, &output_size);

    // failed to execute
    if (output_buffer == NULL)
    {
        http_code = 500;
        write(client->msgsock, HTTP_RESPONSE_500, strlen(HTTP_RESPONSE_500));
        printf("HTTP_CODE: 500\n");
        return http_code;
    }

    http_code = 200;

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

int http_response_default(Client *client)
{
    int http_code = 0;

    http_code = 501;
    write(client->msgsock, HTTP_RESPONSE_501, strlen(HTTP_RESPONSE_501));
    printf("501 Method Not Implemented\n");
    return http_code;
}

int make_http_response(Client *client)
{
	// serve file
    switch (client->header->method)
    {

    	case 1: // 1 is GET
    	{
            http_response_get(client);
            break;
        }

        case 3: // 3 is POST
        {
            http_response_post(client);
            break;
        }

        default: // unimplemented
        {
            http_response_default(client);
            break;
        }

	}

	return 0;
}
