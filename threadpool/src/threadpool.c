/**
 * @file threadpool.c
 * @brief Threadpool implementation file
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "threadpool.h"

#define MAX_NAME_LEN 25

typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown  = 2
} threadpool_shutdown_t;

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

typedef struct {
	void (*function)(void *);
	void *argument;
} threadpool_task_t;

typedef struct thread_task_t
{
	void (*function)(void *);
	void *argument;
}thread_task;

typedef struct thread_t{
	pthread_t tid;
	char thread_name[MAX_NAME_LEN];
	thread_task *task_queue;
}thread;


/**
 *  @struct threadpool
 *  @brief The threadpool struct
 *
 *  @var notify       Condition variable to notify worker threads.
 *  @var threads      Array containing worker threads ID.
 *  @var thread_count Number of threads
 *  @var queue        Array containing the task queue.
 *  @var queue_size   Size of the task queue.
 *  @var head         Index of the first element.
 *  @var tail         Index of the next element.
 *  @var count        Number of pending tasks
 *  @var shutdown     Flag indicating if the pool is shutting down
 *  @var started      Number of started threads
 */
struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  
  int thread_count;
  pthread_t *threads;

  thread *_threads;
  
  int queue_size;
  threadpool_task_t *queue;
  int head;
  int tail;
  int count;
  
  int shutdown;
  int started;
};

/**
 * @function void *threadpool_thread(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
static void *threadpool_thread(void *threadpool);

int threadpool_free(threadpool_t *pool);

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    threadpool_t *pool;
    int i;

    /* TODO: Check for negative or otherwise very big input parameters */

    if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL) {
        goto err;
    }

    /* Initialize */
	pool->shutdown = pool->started = 0;
    
	pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->queue = (threadpool_task_t *)malloc (sizeof(threadpool_task_t) * pool->queue_size);

    /* Allocate thread and task queue */
    pool->thread_count = 0;
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);

    /* Initialize mutex and conditional variable first */
    if((pthread_mutex_init(&(pool->lock), NULL) != 0) ||
       (pthread_cond_init(&(pool->notify), NULL) != 0) ||
       (pool->threads == NULL) ||
       (pool->queue == NULL)) {
        goto err;
    }

    /* Start worker threads */
    for(i = 0; i < thread_count; i++)
	{
		//TODO　set thread attr
        if(pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void*)pool) != 0) {
            threadpool_destroy(pool, 0);
            return NULL;
        }
        pool->thread_count++;
        pool->started++;
    }

    return pool;

 err:
    if(pool) {
        threadpool_free(pool);
    }
    return NULL;
}

int threadpool_add(threadpool_t *pool, void (*function)(void *), void *argument, int flags)
{
    int err = 0;
    int next;

    if(pool == NULL || function == NULL) {
        return threadpool_invalid;
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    next = pool->tail + 1;
    next = (next == pool->queue_size) ? 0 : next;

    do {
        /* Are we full ? */
        if(pool->count == pool->queue_size) {
            err = threadpool_queue_full;
            break;
        }

        /* Are we shutting down ? */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        /* Add task to queue */
        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count += 1;

        /* pthread_cond_broadcast */
        if(pthread_cond_signal(&(pool->notify)) != 0) {
            err = threadpool_lock_failure;
            break;
        }
    } while(0);

    if(pthread_mutex_unlock(&pool->lock) != 0) {
        err = threadpool_lock_failure;
    }

    return err;
}

int threadpool_destroy(threadpool_t *pool, int flags)
{
    if(pool == NULL)  return threadpool_invalid;

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    int i, err = 0;
    do {
        /* Already shutting down */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        pool->shutdown = (flags & threadpool_graceful) ?  graceful_shutdown : immediate_shutdown;

        /* Wake up all worker threads */
        if((pthread_cond_broadcast(&(pool->notify)) != 0) || (pthread_mutex_unlock(&(pool->lock)) != 0)) {
            err = threadpool_lock_failure;
            break;
        }

        /* Join all worker thread */
        for(i = 0; i < pool->thread_count; i++) 
		{
            if(pthread_join(pool->threads[i], NULL) != 0) {
                err = threadpool_thread_failure;
            }
        }
    } while(0);

    /* Only if everything went well do we deallocate the pool */
    if(!err) {
        threadpool_free(pool);
    }
    return err;
}

int threadpool_free(threadpool_t *pool)
{
	if(pool == NULL || pool->started > 0) {
		return -1;
	}

	/* Did we manage to allocate ? */
	if(pool->threads) {
		free(pool->threads);
		free(pool->queue);

		/* Because we allocate pool->threads after initializing the
		   mutex and condition variable, we're sure they're
		   initialized. Let's lock the mutex just in case. */
		
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_cond_destroy(&(pool->notify));
	}
	free(pool);    
	return 0;
}

int threadpool_queue_size(threadpool_t *pool)
{
    if(pool == NULL)  return threadpool_invalid;

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }
	
	int err = 0;
	int queue_size = 0;
	do{
		/* Already shutting down */
		if(pool->shutdown){ 
			err = threadpool_shutdown;
			break;
		}

		queue_size = pool->count;
	}while(0);

	if(pthread_mutex_unlock(&pool->lock) != 0)
	{
		return threadpool_lock_failure;
	}

	if(err != 0) {
		return err;
	}
	
	return queue_size;
}

void threadpool_status(threadpool_t *pool)
{
    if(pool == NULL) { 
        fprintf(stderr, "%s %d %s threadpool is not valid\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        fprintf(stderr, "%s %d %s threadpool lock failure\n", __FILE__, __LINE__, __FUNCTION__);
		return;
    }
	
	/* Already shutting down */
	if(pool->shutdown){ 
		fprintf(stderr, "%s %d %s threadpool shutdown\n", __FILE__, __LINE__, __FUNCTION__);
		if(pthread_mutex_unlock(&pool->lock) != 0)
		{
			fprintf(stderr, "%s %d %s threadpool unlock failure\n", __FILE__, __LINE__, __FUNCTION__);
			return;
		}
		return;
	}
	
	fprintf(stderr, "==============threadpool status============\n");
	fprintf(stderr, "threadpool thread count:%d\n", pool->thread_count);
	fprintf(stderr, "threadpool task cap:%d\n", pool->queue_size);
	fprintf(stderr, "threadpool task count:%d\n", pool->count);
	fprintf(stderr, "============================================\n");
	
	if(pthread_mutex_unlock(&pool->lock) != 0)
	{
        fprintf(stderr, "%s %d %s threadpool unlock failure\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
}

static void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    for(;;)
	{
         /* Lock must be taken to wait on conditional variable */
        pthread_mutex_lock(&(pool->lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), we own the lock. */
        while((pool->count == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if((pool->shutdown == immediate_shutdown) ||
           ((pool->shutdown == graceful_shutdown) &&
            (pool->count == 0))) {
            break;
        }

        /* Grab our task */
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        pool->head += 1;
        pool->head = (pool->head == pool->queue_size) ? 0 : pool->head;
        pool->count -= 1;

        /* Unlock */
        pthread_mutex_unlock(&(pool->lock));

        /* Get to work */
        (*(task.function))(task.argument);
    }
	
	fprintf(stderr, "thread :%u terminated\n", pthread_self());
    pool->started--;

    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);

    return(NULL);
}
