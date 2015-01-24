/*
 *  The implementation of DB System.
 *  Copyright (C)  2008 - 2015 
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 *  02110-1301  USA
 *
 *  Author e-mail: likunemail@163.com 
 *                 likunemail@gmail.com
 *  Last Modified: 01/24/2015, 08:08:58 PM
 *  Filename:      test.c  
 */

#define THREAD 5
#define QUEUE  1000

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "../src/threadpool.h"

int tasks = 0, done = 0;
pthread_mutex_t lock;

void dummy_task_1(void *arg) 
{
	//fprintf(stderr, "%s  %d  %s\n", __FILE__, __LINE__, __FUNCTION__); 
    usleep(10000);
    pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

void dummy_task_2(void *arg) 
{
	fprintf(stderr, "%s  %d  %s\n", __FILE__, __LINE__, __FUNCTION__); 
    usleep(10000);
    pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

void dummy_task_3(void *arg) 
{
	fprintf(stderr, "%s  %d  %s\n", __FILE__, __LINE__, __FUNCTION__); 
    usleep(10000);
    pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

void dummy_task_4(void *arg) 
{
	fprintf(stderr, "%s  %d  %s\n", __FILE__, __LINE__, __FUNCTION__); 
    usleep(10000);
    pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

void dummy_task_5(void *arg) 
{
	fprintf(stderr, "%s  %d  %s\n", __FILE__, __LINE__, __FUNCTION__); 
    usleep(10000);
    pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
    threadpool_t *pool;

    pthread_mutex_init(&lock, NULL);

    assert((pool = threadpool_create(THREAD, QUEUE, 0)) != NULL);
    fprintf(stderr, "Pool started with %d threads and queue size of %d\n", THREAD, QUEUE);

	do {
		if(threadpool_add(pool, &dummy_task_1, NULL, 0) == 0)
		{
			pthread_mutex_lock(&lock);
			tasks++;
			pthread_mutex_unlock(&lock);
			break;
		}
	}while(1);
	do {
		if(threadpool_add(pool, &dummy_task_2, NULL, 0) == 0)
		{
			pthread_mutex_lock(&lock);
			tasks++;
			pthread_mutex_unlock(&lock);
			break;
		}
	}while(1);
	do {
		if(threadpool_add(pool, &dummy_task_3, NULL, 0) == 0)
		{
			pthread_mutex_lock(&lock);
			tasks++;
			pthread_mutex_unlock(&lock);
			break;
		}
	}while(1);
	do {
		if(threadpool_add(pool, &dummy_task_4, NULL, 0) == 0)
		{
			pthread_mutex_lock(&lock);
			tasks++;
			pthread_mutex_unlock(&lock);
			break;
		}
	}while(1);
	do {
		if(threadpool_add(pool, &dummy_task_5, NULL, 0) == 0)
		{
			pthread_mutex_lock(&lock);
			tasks++;
			pthread_mutex_unlock(&lock);
			break;
		}
	}while(1);
	
	int queue_size = threadpool_queue_size(pool);
    fprintf(stderr, "Added %d tasks,  queue_size:%d\n", tasks, queue_size);
	
	threadpool_status(pool);

	while(threadpool_add(pool, &dummy_task_1, NULL, 0) == 0) 
	{
        pthread_mutex_lock(&lock);
        tasks++;
        pthread_mutex_unlock(&lock);
    }
	
	queue_size = threadpool_queue_size(pool);
    fprintf(stderr, "Added %d tasks,  queue_size:%d\n", tasks, queue_size);
	threadpool_status(pool);
    
	//while((tasks / 2) > done) 
	while(tasks > done) 
	{
        usleep(10000);
    }

	threadpool_status(pool);
    assert(threadpool_destroy(pool, 0) == 0);
    fprintf(stderr, "Did %d tasks  %d\n", done, tasks);

    return 0;
}
