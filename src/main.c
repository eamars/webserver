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

#include "parser.h"
#include "datetime.h"
#include "client.h"


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



void handle_request(int msgsock)
{
	Client *client = (Client *) malloc(sizeof(Client));

	client->msgsock = msgsock;

	get_peer_information(client);

	read_http_request(client);

	make_http_response(client);

	free(client->header);
	free(client);
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
	// ignore the return value from child
	signal(SIGCHLD, SIG_IGN);

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
