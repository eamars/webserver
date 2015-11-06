#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#include "config.h"
#include "worker.h"
#include "client.h"
#include "error.h"


struct QueueStruct
{
    int enqueue;
    int dequeue;
};

#define TIMEOUT 2
#define ERROR 1
#define SUCCESS 0
static void sig_alrm(int signo){
	exit(TIMEOUT);
}


void wait_workers(ThreadPool *pool, ThreadConfig *thread_config)
{
    int i;

    // throw termination marks
    for (i = 0; i < pool->num_threads; i++)
    {
        queue_put(thread_config->queue, NULL);
    }

    // wait for threads to terminate
    for (i = 0; i < pool->num_threads; i++)
    {
        pthread_join(pool->thread_pool[i], NULL);
    }

    // free allocated thread pool
    if (pool->thread_pool)
    {
        free(pool->thread_pool);
    }
}

void *worker_thread(void *args)
{
    int                         tid;
    int                         msgsock;
    void                        *item;
    int                         pid;
    int                         status;
    int                         tcp_timeout;
    char 					    value[MAX_KEY_LEN];
    int                         rc;

    ThreadConfig                *thread_config;
    thread_argument_wapper      *wapper;
    Configuration               *config;
    Queue                       *queue;
    pthread_mutex_t             mutex;

    // extract arguments
    wapper = (thread_argument_wapper *) args;

    // get thread id
    tid = wapper->thread_id;
    printf("Worker thread [%d] starts\n", tid);
    fflush(stdout);

    // get thread config
    thread_config = wapper->thread_config;
    config = thread_config->config;
    queue = thread_config->queue;
    mutex = thread_config->mutex;

    // get tcp_timeout from config
    memset(value, 0, MAX_VALUE_LEN);
    rc = config_get_value(config, "tcp_timeout", value);
	if (!rc)
	{
		handle_error("tcp_timeout");
	}
	tcp_timeout = atoi(value);

    // get sock from queue
    while (1)
    {
        item = queue_get(queue);
        if (item == NULL)
        {
            break;
        }
        printf("Worker thread [%d] get: %p\n", tid, item);
        fflush(stdout);

        msgsock = *(int *)item;

        // free allocated msgsock
        free(item);

        pid = fork();
        if (pid < 0)
        {
            handle_error("fork");
        }
        else if (pid == 0)
        {
            if (signal(SIGALRM, sig_alrm) == SIG_ERR){
    			handle_error("signal");
    		}

            alarm(tcp_timeout); // setup 10s
            handle_request(config, msgsock);
            alarm(0);

            exit(SUCCESS);
        }
        else
        {
            close(msgsock);
            waitpid(pid, &status, 0);
        }

        printf("Worker thread [%d] served complete with exit code %d\n", tid, WEXITSTATUS(status));
    }

    printf("Worker thread [%d] terminates\n", tid);
    fflush(stdout);

    // free allocated wapper
    free(wapper);

    return NULL;

}

Queue *queue_init()
{
    int pfd[2];
    Queue *queue = (Queue *) malloc (sizeof(Queue));

    if (pipe(pfd) < 0)
    {
        handle_error("pipe");
    }

    queue->enqueue = pfd[1];
    queue->dequeue = pfd[0];

    return queue;
}

void queue_free(Queue *queue)
{
    close(queue->enqueue);
    close(queue->dequeue);

    free(queue);
}

void queue_put(Queue *queue, void *item)
{
    write(queue->enqueue, &item, sizeof(item));
}

void *queue_get(Queue *queue)
{
    void *item;

    read(queue->dequeue, &item, sizeof(item));

    return item;
}
