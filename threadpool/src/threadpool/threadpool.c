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
#define DEFAULT_THREAD_NUM 10
#define DEFAULT_QUEUE_NUM 30

typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown  = 2
} threadpool_shutdown_t;

typedef struct
{
	void (*function)(void *);
	void *argument;
}thread_task_t;

typedef struct 
{
    int head;
    int tail;
    int queue_size;
	thread_task_t *task_queue;
    
    pthread_mutex_t lock;
    pthread_cond_t notify;
}task_queue_t;

typedef struct{
	pthread_t tid;
	char thread_name[MAX_NAME_LEN];
    task_queue_t *task_queue;
}thread_t;

struct threadpool_t {
    pthread_mutex_t lock;
    
    int thread_count;
    thread_t *threads;

    int shutdown;
    int started;
};

static void *_threadpool_thread(void *threadpool);
static int get_lowload_thread(threadpool_t *pool);

static int _thread_free(thread_t thread);
int threadpool_free(threadpool_t *pool);

static int _queue_capacity(task_queue_t *queue);
static int _push_queue(task_queue_t *queue, thread_task_t task);
static thread_task_t* _pop_queue(task_queue_t *queue);


threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    /* Check for negative or otherwise very big input parameters */
    if(thread_count <= 0) { 
        thread_count = DEFAULT_THREAD_NUM;
    }

    int i = 0;
    threadpool_t *pool;
    if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL) {
        goto err;
    }

    /* Initialize */
	pool->shutdown = pool->started = 0;


    /* Initialize mutex and conditional variable first */
    if((pthread_mutex_init(&(pool->lock), NULL) != 0) ||
       (pthread_cond_init(&(pool->notify), NULL) != 0) ||
       (pool->threads == NULL) ||
       (pool->queue == NULL)) {
        goto err;
    }
    
    pool->queue = (threadpool_task_t *)malloc (sizeof(threadpool_task_t) * pool->queue_size);

    /* Allocate thread and task queue */
    pool->thread_count = 0;
    pool->threads = (thread_t *)malloc(sizeof(thread_t) * thread_count);
    if(pool->threads == NULL) {
        goto err;
    }
    
    /* Start worker threads */
    for(i = 0; i < thread_count; i++)
	{
		//TODO　set thread attr
        thread_create(&pool->threads[i], pool);
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
    if(pool == NULL || function == NULL)  return threadpool_invalid;
    
    /* Are we shutting down ? */
    if(pool->shutdown)  return threadpool_shutdown;
    
    int index = get_lowload_thread(pool);
    if(index == -1) return threadpool_invalid;
    
    thread_t *thread = pool->threads[index];
    task_queue_t *queue = thread->task_queue;
    if(queue == NULL)  return threadpool_invalid;
    
    if(pthread_mutex_lock(&(queue->lock)) != 0)  return threadpool_lock_failure;
    
    int err = 0;
    do{
        /* Add task to queue */
        thread_task_t task = {function, argument};
       if(_push_queue(queue, task) != 0) {
            err = threadpool_lock_failure;
            break;
       }
        
        /* pthread_cond_broadcast */
        if(pthread_cond_signal(&(pool->notify)) != 0) {
            err = threadpool_lock_failure;
            break;
        }
    }while(0);

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

int threadpool_free(threadpool_t *pool)
{
	if(pool == NULL || pool->started > 0) {
		return -1;
	}

	/* Did we manage to allocate ? */
	if(pool->threads)
    {
		int i = 0;
        for(i=0; i<thread_count; i++)
        {
            thread_free(pool->threads[i]);
        }

        free(pool->threads);
        
        /* Because we allocate pool->threads after initializing the
         * mutex and condition variable, we're sure they're
         * initialized. Let's lock the mutex just in case. 
        **/
		pthread_mutex_lock(&(pool->lock));
		pthread_mutex_destroy(&(pool->lock));
		pthread_cond_destroy(&(pool->notify));
	}
	
    free(pool);    
	return 0;
}

static void *_threadpool_thread(void *threadpool)
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

static void *_thread_work(void *arg)
{
    if(arg == NULL) {
        threadpool_destroy(pool, 0);
        exit(-1); 
    }

    thread_t *thread = (thread_t *)arg;
    thread_task_t task;
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

static int get_lowload_thread(threadpool_t *pool)
{
    if(pool == NULL){
        return -1;
    }

    return 1;
}

static int thread_create(thread_t *thread, threadpool_t *pool, int queue_size)
{
    if(pthread_create(thread->tid, NULL, _thread_work, (void*)thread) != 0) {
        threadpool_destroy(pool, 0);
        return -1;
    }
    
    if(queue_size <= 0){
        queue_size = DEFAULT_QUEUE_NUM;
    }
    
    snprintf(thread->thread_name, "thread:%u", pthread_self());

    return 0;
}

static int thread_free(thread_t thread)
{
    if(thread == NULL) return -1;
    
    //TODO
    return 0;
}

static int _queue_capacity(task_queue_t *queue)
{
    if(queue == NULL) return 0;
    
    if(queue->head <= queue->tail){
        return queue->queue_size - (queue->tail - queue->head); 
    } 
    else
    {
        return queue->head - queue->tail;
    }
}

static int _push_queue(task_queue_t *queue, thread_task_t task)
{
    if(queue == NULL) return -1;
    if(_queue_capacity(queue) <= 0) return -1;

    int next = queue->tail + 1;
    next = (next > queue->queue_size) : 0 ?next;

    queue->task_queue[next] = task;
    queue->tail = next;

    return 0;
}

static thread_task_t* _pop_queue(task_queue_t *queue)
{
    if(queue == NULL) return NULL;
    if(_queue_capacity(queue) == queue->queue_size) return NULL;

    thread_task_t *task = &queue->task_queue[queue->head];
    int next = queue->head + 1;
    next = (next > queue->queue_size) : 0 ?next;
    queue->head = next;

    return task;
}

