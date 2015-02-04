/**
 * @file threadpool.c
 * @brief Threadpool implementation file
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "threadpool.h"

#define DEFAULT_THREAD_NUM 10
#define DEFAULT_QUEUE_SIZE 30

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
}task_queue_t;

typedef struct{
	pthread_t tid;
    int pool_idx;
    int running;

    int task_num;
    task_queue_t *task_queue;
    
    pthread_mutex_t lock;
    pthread_cond_t notify;
}thread_t;

struct threadpool_t {
    int thread_count;
    thread_t *threads;
    
    pthread_mutex_t lock;
    int shutdown;
    int started;
};

struct thread_params_t{
    int idx;
    struct threadpool_t *pool;
};

void *_threadpool_thread(void *threadpool);
int get_lowload_thread(threadpool_t *pool);

int _thread_free(thread_t thread);
int threadpool_free(threadpool_t *pool);

int _queue_capacity(task_queue_t *queue);
int _push_queue(task_queue_t *queue, thread_task_t task);
thread_task_t* _pop_queue(task_queue_t *queue);

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    /* Check for negative or otherwise very big input parameters */
    if(thread_count <= 0)  thread_count = DEFAULT_THREAD_NUM;
    if(queue_size <= 0) queue_size = DEFAULT_QUEUE_SIZE;

    threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    if(pool == NULL) goto err;

    /* Initialize */
	pool->shutdown = pool->started = 0;
    
    if((pthread_mutex_init(&(pool->lock), NULL) != 0)) goto err;

    /* Allocate thread and task queue */
    pool->thread_count = thread_count;
    pool->threads = (thread_t *)malloc(sizeof(thread_t) * thread_count);
    if(pool->threads == NULL)  goto err;
    
    /* Start worker threads */
    int i = 0;
    for(i = 0; i < pool->thread_count; i++)
	{
        thread_create(pool, queue_size, i);
        pool->started++;
    }

    return pool;

 err:
    if(pool)  threadpool_free(pool);
    return NULL;
}

int threadpool_add_task(threadpool_t *pool, void (*function)(void *), void *argument, int flags)
{
    if(pool == NULL || function == NULL)  return threadpool_invalid;
    
    /* Are we shutting down ? */
    if(pool->shutdown)  return threadpool_shutdown;
    
    int idx = get_lowload_thread(pool);
    if(idx == -1) return threadpool_invalid;
    
    thread_t *thread = &pool->threads[idx];
    task_queue_t *queue = thread->task_queue;
    if(queue == NULL) return threadpool_invalid;
    
    pthread_mutex_lock(&(thread->lock));
    
    int err = 0;
    do{
        /* Add task to queue */
        thread_task_t task = {function, argument};
       if(_push_queue(queue, task) != 0) 
       {
            err = threadpool_lock_failure;
            break;
       }

       thread->task_num++;

    }while(0);

    pthread_mutex_unlock(&pool->lock);
    return err;
}

int threadpool_destroy(threadpool_t *pool, int flags)
{
    if(pool == NULL)  return threadpool_invalid;
    return 0;
 }


int threadpool_free(threadpool_t *pool) { return 0; }
int threadpool_queue_size(threadpool_t *pool) { }
void threadpool_status(threadpool_t *pool) { }
void *_threadpool_thread(void *threadpool) { return(NULL); }

void *_thread_work(void *thread_params)
{
    if(thread_params == NULL)  exit(-1); 

    struct thread_params_t *params = (struct thread_params_t*)thread_params;
    int idx = params->idx;
    threadpool_t *pool = params->pool;
    thread_t *thread = &pool->threads[idx];

    pthread_mutex_lock(&(thread->lock));
    thread->running = 1;
    pthread_mutex_unlock(&(thread->lock));

    pthread_mutex_lock(&(pool->lock));
    pool->started++;
    pthread_mutex_unlock(&(pool->lock));

    while(1)
    {
        /* Lock must be taken to wait on conditional variable */
        pthread_mutex_lock(&(thread->lock));

        while(thread->running == 1 && thread->task_num == 0)
        {
            pthread_cond_wait(&(thread->notify), &(thread->lock));
        }

        //terminate this thread
        if(thread->running == 0) break;

        //no task to do
        if(thread->task_num == 0) 
        {
            pthread_mutex_unlock(&(thread->lock));
            continue;
        }

        /* Unlock */
        pthread_mutex_unlock(&(thread->lock));

        /* Grab our task */
        thread_task_t *task = _pop_queue(thread->task_queue);
        if(task != NULL && task->function != NULL)
        {
            (*(task->function))(task->argument);
        }
        thread->task_num--;
    }

    fprintf(stderr, "thread :%u terminated\n", (unsigned)pthread_self());
    pthread_mutex_unlock(&(thread->lock));

    pthread_mutex_lock(&(pool->lock));
    pool->started--;
    pthread_mutex_unlock(&(pool->lock));

    pthread_exit(NULL);

    return(NULL);
}

int thread_create(threadpool_t *pool, int queue_size, int idx)
{
    task_queue_t *task_queue = (task_queue_t *)malloc (sizeof(task_queue_t) * queue_size);
    if(task_queue == NULL) return -1;

    task_queue->head = task_queue->tail = 0;
    task_queue->queue_size = queue_size;

    thread_t *thread = &pool->threads[idx];
    thread->pool_idx = idx;
    thread->task_num = 0;
    thread->task_queue = task_queue;

    /* Initialize mutex and conditional variable */
    if((pthread_mutex_init(&(thread->lock), NULL) != 0) || (pthread_cond_init(&(thread->notify), NULL) != 0) ) return -1;

    //you canset thread attr by yourself
    struct thread_params_t *params = malloc(sizeof(struct thread_params_t));
    if(params == NULL) return -1;

    params->idx = idx;
    params->pool = pool;
    if(pthread_create(&thread->tid, NULL, _thread_work, (void*)params) != 0)  return -1;

    return 0;
}

int thread_free(thread_t* thread)
{
    if(thread == NULL) return -1;
    return 0;
}


int _queue_capacity(task_queue_t *queue)
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

int _push_queue(task_queue_t *queue, thread_task_t task)
{
    if(queue == NULL) return -1;
    if(_queue_capacity(queue) <= 0) return -1;

    int next = queue->tail + 1;
    next = (next > queue->queue_size) ? 0 : next;

    queue->task_queue[next] = task;
    queue->tail = next;

    return 0;
}

thread_task_t* _pop_queue(task_queue_t *queue)
{
    if(queue == NULL) return NULL;
    if(_queue_capacity(queue) == queue->queue_size) return NULL;

    thread_task_t *task = &queue->task_queue[queue->head];
    int next = queue->head + 1;
    next = (next > queue->queue_size) ? 0 : next;
    queue->head = next;

    return task;
}

int get_lowload_thread(threadpool_t *pool)
{
    if(pool == NULL) return -1;
    return 1;
}
