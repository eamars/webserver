// Idea comes from
// http://stackoverflow.com/questions/11254037/how-to-know-when-the-http-headers-part-is-ended

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
#include <signal.h>

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define READ_SZ 16


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

void parse_raw_http_header(int s)
{
	ssize_t 			sz;
	size_t 				readed;
	char 				buffer[READ_SZ];
	char 				*data;
	char 				*header_end;

	// reset variables
	readed = 0;
	data = NULL;
	header_end = NULL;

	while ((sz = read(s, buffer, READ_SZ-1)) != 0)
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
				printf("header: %s\n", data);
			}

		}

	}

}

void handle_request(int msgsock)
{
	printf("New Client\n");
    parse_raw_http_header(msgsock);
    fflush(stdout);
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

int main(void)
{
    int sock;
    int msgsock;


    sock = listen_on(8080);

    while (1)
    {
        msgsock = accept_connection(sock);
        handle_fork(msgsock);
    }

    close(sock);
    return 0;
}
