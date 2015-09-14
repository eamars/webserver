#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <http_parser.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

#include "parser.h"

#define MAX_SZ 1024
#define SERVER_PORT 8080

// The multimedia content is still a problem
const char *HTTP_RESPONSE_TEMPLATE =
"HTTP/1.1 200 OK\r\n"
"Server: ENCE360-WebServer/1.0\r\n"
"Connection: close\r\n"
"Content-Length %ld\r\n"
"\r\n"
"%s\r\n"
"\r\n";

const char *HTTP_RESPONSE_404 =
"HTTP/1.1 404 Not Found\r\n"
"\r\n"
"404 - Page Not Found\r\n"
"\r\n";



int listen_on(int port)
{
	int sock;
	struct sockaddr_in sa;
	int ret;

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	ret = bind(sock, (struct sockaddr *) &sa, sizeof(sa));
	if (ret < 0)
	{
		perror("bind");
		exit(-1);
	}

	ret = listen(sock, 5);
	if (ret < 0)
	{
		perror("listen");
		exit(-1);
	}

	return sock;
}

int accept_connection(int sock)
{
	struct sockaddr_in caller;
	socklen_t length = sizeof(caller);

	int msgsock = accept(sock, (struct sockaddr *) &caller, &length);

	return msgsock;
}

int get_line(int msgsock, char *buf, int size)
{

	int sz;
	int i = 0;
	int ch;
	int state = 0;

	while ((i < MAX_SZ - 1) && state != 4)
	{
		sz = read(msgsock, &ch, 1);
		if (sz > 0)
		{
			if (ch == 'r')
			{
				switch (state) {
					case 0:
						state = 1;
						break;
					case 1:
						state = 0;
						break;
					case 2:
						state = 3;
						break;
					case 3:
						state = 0;
						break;
				}
			}
			else if (ch == '\n')
			{
				switch (state) {
					case 0:
						state = 0;
						break;
					case 1:
						state = 2;
						break;
					case 2:
						state = 0;
						break;
					case 3:
						state = 4;
						break;
				}
			}
			buf[i++] = ch;
		}
		else
		{
			break;
		}
	}
	buf[i] = 0;
	return i;
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

int make_response(int msgsock, char *buffer, int size)
{
	// parse http header
	int rc;

	http_header_t *header = (http_header_t *) malloc (sizeof(http_header_t));
	rc = parse(header, buffer, size);

	// serve file
	if (header->method == 1) // 1 is GET
	{
		char *file_buffer = NULL;
		char path[MAX_SZ];
		size_t file_size;

		sprintf(path, "./html%s", header->url);
		printf("PATH: %s\n", path);
		file_buffer = read_file(path, &file_size);
			printf("FILE: %s\n", file_buffer);

		if (file_buffer == NULL)
		{
			write(msgsock, HTTP_RESPONSE_404, strlen(HTTP_RESPONSE_404));
			printf("404 Not Found\n");
		}
		else
		{
			char buf[file_size + strlen(HTTP_RESPONSE_TEMPLATE) * 2];
			memset(buf, 0, sizeof(buf));
			sprintf(buf, HTTP_RESPONSE_TEMPLATE, file_size, file_buffer);

			printf("SEND: \n----------\n%s\n----------\n", buf);
			write(msgsock, buf, sizeof(buf));
		}

		// safely free file buffer
		if (file_buffer != NULL)
		{
			free(file_buffer);
		}

	}


	free(header);
	return 0;
}

void handle_request(int msgsock)
{
	int sz;
	char buffer[MAX_SZ];

	memset(buffer, 0, MAX_SZ);

	sz = read(msgsock, buffer, MAX_SZ);
	printf("RECV:\n----------\n%s\n----------\n", buffer);

	make_response(msgsock, buffer, sz);


	close(msgsock);
}

void handle_fork(int msgsock)
{
	int pid;

	pid = fork();

	if (pid < 0)
	{
		perror("fork");
		exit(-1);
	}
	else if (pid == 0)
	{
		handle_request(msgsock);
		exit(0);
	}
	else
	{
		close(msgsock);
	}
}

int main(int argc, char **argv)
{
	printf("This is the server with pid %d listening on port %d\n", getpid(), atoi(argv[1]));

	int sock;
	int msgsock;

	sock = listen_on(atoi(argv[1]));

	while (1)
	{
		msgsock = accept_connection(sock);
		handle_fork(msgsock);
	}

	close(sock);
	exit(0);
}
