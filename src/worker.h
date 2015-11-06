#ifndef WORKER_H_
#define WORKER_H_

#include <pthread.h>

// concurrent queue interface
typedef struct QueueStruct Queue;

typedef struct ThreadConfigStruct
{
    Queue *queue;
    Configuration *config;
    pthread_mutex_t mutex;
} ThreadConfig;

typedef struct ThreadPoolStruct
{
    pthread_t *thread_pool;
    int num_threads;
} ThreadPool;

typedef struct
{
    ThreadConfig *thread_config;
    int thread_id;
} thread_argument_wapper;


void wait_workers(ThreadPool *pool, ThreadConfig *thread_config);

void *worker_thread(void *args);

/*
 * queue_put:
 *
 * Place an item into the concurrent queue.
 *
 * If there is no space available then queue_put will
 * block until a space becomes available when it will
 * put the item into the queue and immediately return.
 *
 * Uses void* to hold an arbitrary type of item,
 * it is the users responsibility to manage memory
 * and ensure it is correctly typed.
 *
 */
Queue *queue_init();

/*
 * queue_free:
 *
 * Free a concurrent queue and associated memory.
 * NOTE: A user should not free a queue until any
 * users are finished. Any calling queue_free() while
 * any consumer/producer is waiting on queue_put or queue_get
 * will cause queue_free to print an error and exit the program.
 *
 */
void queue_free(Queue *queue);

/*
 * queue_put:
 *
 * Place an item into the concurrent queue.
 *
 * If there is no space available then queue_put will
 * block until a space becomes available when it will
 * put the item into the queue and immediately return.
 *
 * Uses void* to hold an arbitrary type of item,
 * it is the users responsibility to manage memory
 * and ensure it is correctly typed.
 *
 */
void queue_put(Queue *queue, void *item);

/*
 * queue_get:
 *
 * Get an item from the concurrent queue.
 *
 * If there is no item available then queue_get
 * will block until an item becomes avaible when
 * it will immediately return that item.
 *
 */
void *queue_get(Queue *queue);

#endif
