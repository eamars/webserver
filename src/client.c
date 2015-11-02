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
#include <http_parser.h>
#include <sys/wait.h>
#include <errno.h>

#include "client.h"
#include "datetime.h"
#include "parser.h"
#include "config.h"

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define MAX_SZ 8192
#define READ_SZ 4096
#define MAX_QUERY_SZ 255

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

    printf("IP: %s\n", client->ipstr);
    while ((sz = read(client->msgsock, buffer, READ_SZ-1)) != 0)
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
				printf("RECV:\n----------\n%s\n----------\n", data);
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


int http_response_get(Configuration *config, Client *client)
{
    int rc;
    int http_code = 0;
    char datetime[MAX_DATETIME_LENGTH];
    char path[MAX_SZ];
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

char *execute_cgi(char *path, size_t *output_size, Client *client)
{
    int input_fd[2];
    int output_fd[2];
    int pid;
    int status;
    char *output_buffer = NULL;
    int content_length = -1;

    // get content length
    for (int i = 0; i < client->header->num_fields; i++)
    {
        if (strcasecmp(client->header->fields[i], "Content-Length") == 0)
        {
            content_length = atoi(client->header->values[i]);
            break;
        }
    }
    if (content_length == -1)
    {
        fprintf(stderr, "Unable to http request header error: not a valid Content-Length");
        return NULL;
    }

    // open pipe for executing cgi script
    if (pipe(input_fd) < 0 || pipe(output_fd) < 0)
    {
        perror("pipe");
        return NULL;
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return NULL;
    }
    else if (pid == 0)
    {
        // child code
        char method_env[MAX_QUERY_SZ];
        char query_env[MAX_QUERY_SZ];
        char length_env[MAX_QUERY_SZ];

        // copy query
        // TODO: GET and POST are able to execute cgi script. I need to extract query
        // start with ?
        sprintf(method_env, "REQUEST_METHOD=%s", http_method_str(client->header->method));
        putenv(method_env);

        sprintf(query_env, "QUERY_STRING=%s", "");
        putenv(query_env);

        sprintf(length_env, "CONTENT_LENGTH=%d", content_length);
        putenv(length_env);


        // redirect pipe
        dup2(output_fd[1], STDOUT_FILENO);
        dup2(input_fd[0], STDIN_FILENO);
        close(output_fd[0]);
        close(input_fd[1]);

        execl(path, path, NULL);
        perror("execl");
        exit(-1);
    }
    else
    {
        // parent code
        char strbuf[READ_SZ];
    	*output_size = 0;
        int sz;

        // close pipe
        close(output_fd[1]);
        close(input_fd[0]);

        // write form data to CGI
        write(input_fd[1], client->header->body, strlen(client->header->body));
        write(input_fd[1], client->payload, strlen(client->payload));

        // wait for child to complete
        waitpid(pid, &status, 0);

    	memset(strbuf, 0, READ_SZ);
    	while ((sz = read(output_fd[0], strbuf, READ_SZ)) != 0)
    	{
    		if (sz < 0)
    		{
    			perror("read");
    			return NULL;
    		}

    		// append
    		output_buffer = realloc(output_buffer, *output_size + sz);
    		memcpy(output_buffer + *output_size, strbuf, sz);
    		*output_size += sz;

    		memset(strbuf, 0, READ_SZ);
    	}
    	close(output_fd[0]);
        close(input_fd[1]);
    }

    return output_buffer;

}
int http_response_post(Configuration *config, Client *client)
{
    int rc;
    int http_code = 0;
    char datetime[MAX_DATETIME_LENGTH];
    char path[MAX_SZ];
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
