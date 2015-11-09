#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "parser.h"
#include "datetime.h"
#include "client.h"
#include "config.h"
#include "http_parser.h"
#include "worker.h"


static const char *usage = "webhttpd.out site_package_folder";

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
		handle_error("bind");
	}

	ret = listen(sock, 5);
	if (ret < 0)
	{
		handle_error("listen");
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


int start(char *path)
{
	int 					pid;
	int 					rc;
	char 					config_path[MAX_PATH_SZ];
	char 					value[MAX_KEY_LEN];
	int 					port;
	int 					sock;
	int 					num_workers;

	Configuration 			*config;
	Queue 					*queue;
	ThreadPool 				pool;
	ThreadConfig 			thread_config;


	/* Store PID file and load config*/
	pid = getpid();

	// init configuration
	sprintf(config_path, "%s/site-config", path);
	config = config_init(config_path);
	if (config == NULL)
	{
		handle_error("config_init");
	}

	// read config
	rc = config_load(config);
	if (rc < 0)
	{
		handle_error("config_load");
	}

	// read port number
	memset(value, 0, MAX_VALUE_LEN);
	rc = config_get_value(config, "server_port", value);
	if (!rc)
	{
		handle_error("server_port");
	}
	port = atoi(value);

	// read number of workers
	memset(value, 0, MAX_VALUE_LEN);
	rc = config_get_value(config, "spawn_workers", value);
	if (!rc)
	{
		handle_error("spawn_workers");
	}
	num_workers = atoi(value);

	// create queue
	queue = queue_init();
	thread_config.queue = queue;
	thread_config.config = config;

	// spawn worker
	pool.thread_pool = (pthread_t *) malloc (num_workers * sizeof(pthread_t));
	pool.num_threads = num_workers;

	for (int i = 0; i < num_workers; i++)
	{
		thread_argument_wapper *wapper = (thread_argument_wapper *) malloc (sizeof(thread_argument_wapper));
		wapper->thread_config = &thread_config;
		wapper->thread_id = i;

		pthread_create(&pool.thread_pool[i], NULL, worker_thread, wapper);
	}

	/* Start server */
	sock = listen_on(port);

	// print out server status
	printf("*webhttpd starts with instance %s with pid %d listening on port %d*\n", path, pid, port);

	while (1)
	{
		int *msgsock;

		msgsock = (int *) malloc (sizeof(int));

		*msgsock = accept_connection(sock);

		if (*msgsock < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else
			{
				handle_error("accept");
			}
		}

		queue_put(queue, msgsock);

	}

	config_destroy(config);
	close(sock);
	return 0;
}

int main(int argc, char **argv)
{
	int rc;

	/* Load configuration */
	// ignore the return value from child
	// signal(SIGCHLD, SIG_IGN);

	// test if the user enter the correct arguments
	if (argc != 2)
	{
		fprintf(stderr, "Invalid arguments\nUsage: %s\n", usage);
		return -1;
	}
	return start(argv[1]);
}
