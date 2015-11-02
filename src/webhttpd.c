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
#include "config.h"
#include "pidlock.h"

#define MAX_PATH_SZ 256
static const char *usage = "webhttpd.out [\x1B[32mstart\033[0m|\x1B[32mstop\033[0m] site_package_folder";

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



void handle_request(Configuration *config, int msgsock)
{
	Client *client = (Client *) malloc(sizeof(Client));

	client->msgsock = msgsock;

	get_peer_information(client);

	handle_http_request(config, client);

	close(msgsock);
}

void handle_fork(Configuration *config, int msgsock)
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
		handle_request(config, msgsock);
		exit(0);
	}
	else
	{
		close(msgsock);
	}
}

int start(char *path)
{
	int pid;
	int rc;

	// let child to execute the job
	pid = fork();
	if (pid < 0)
	{
		printf("\x1B[31mFailed to fork subprocess\033[0m\n");
		exit (-1);
	}
	else if (pid == 0)
	{
		/* Store PID file and load config*/
		// write pid to log file for killing tasks later
		pid = getpid();
		rc = write_pid(path, pid);
		if (rc)
		{
			remove_pid(path);
			return rc;
		}

		// try to read config file
	    char config_path[MAX_PATH_SZ];
		char value[MAX_KEY_LEN];
		Configuration *config;
		int port;

		// init configuration
		sprintf(config_path, "%s/site-config", path);
		config = config_init(config_path);
		if (config == NULL)
	    {
	        fprintf(stderr, "Unable to allocate memory\n");
	        exit(-1);
	    }

		// read config
		rc = config_load(config);
		if (rc < 0)
		{
			fprintf(stderr, "Unable to read configuration file: %s\n", config_path);
			exit(-1);
		}

		memset(value, 0, MAX_VALUE_LEN);
	    rc = config_get_value(config, "server_port", value);
		if (!rc)
		{
			fprintf(stderr, "Unable to read port number from: %s\n", config_path);
			exit(-1);
		}
		port = atoi(value);

		// print out server status
		printf("*webhttpd starts with instance %s with pid %d listening on port %d*\n", path, pid, port);

		/* Start server */
		int sock;
		int msgsock;

		sock = listen_on(port);

		while (1)
		{
			msgsock = accept_connection(sock);
			handle_fork(config, msgsock);
		}

		config_destroy(config);
		close(sock);
		return 0;
	}
	else
	{
		return 0;
	}

}

int stop(char *path)
{
	int pid;
	char cmd[512];

	// read pid from pid lock
	pid = read_pid(path);
	if (pid < 0)
	{
		return pid;
	}

	// terminate the task
	// kill both child and parent task
	sprintf(cmd, "pkill -TERM -P %d", pid);
	system(cmd);
	sprintf(cmd, "kill -TERM %d", pid);
	system(cmd);
	// kill(pid, SIG_TERM);

	printf("\x1B[32mwebhttpd with instance %s\033[0m is terminated\n", path);

	remove_pid(path);

	return 0;
}

int main(int argc, char **argv)
{
	int rc;

	/* Load configuration */
	// ignore the return value from child
	signal(SIGCHLD, SIG_IGN);

	// test if the user enter the correct arguments
    if (argc != 3)
    {
        fprintf(stderr, "Invalid arguments\nUsage: %s\n", usage);
        return -1;
    }
	if (!strcmp(argv[1], "start"))
	{
		return start(argv[2]);
	}
	else if (!strcmp(argv[1], "stop"))
	{
		return stop(argv[2]);
	}
	else
	{
		fprintf(stderr, "Invalid arguments\nUsage: %s\n", usage);
		return -1;
	}
}
