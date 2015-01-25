/*
 *  The implementation of DB System.
 *  Copyrighashtable (C)  2008 - 2015 
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#ifndef  __WITH_MURMUR
#define  __WITH_MURMUR 
#endif

#ifndef _MULTI_THREAD
#define _MULTI_THREAD
#endif

#include "../hashtable/src/timer.h"
#include "../threadpool/src/threadpool.h"
#include "../hashtable/src/hashtable.h"

int tasks = 0, done = 0;
pthread_mutex_t lock;

hash_table hashtable;

#define THREAD 1
#define QUEUE  1000

#define KEY_SIZE 25
#define VALUE_SIZE 50
#define TEST_NUM 1000


void do_task()
{
    char *s1 = (char*)"teststring 1";
    char *s2 = (char*)"teststring 2";
    char *s3 = (char*)"teststring 3";

/*
    hashtable_insert(&hashtable, s1, strlen(s1)+1, s2, strlen(s2)+1);
    size_t value_size;
    char *got = hashtable_get(&hashtable, s1, strlen(s1)+1, &value_size);

    fprintf(stderr, "Value size: %zu\n", value_size);
    fprintf(stderr, "Got: {\"%s\": \"%s\"}\n", s1, got);

    fprintf(stderr, "Replacing {\"%s\": \"%s\"} with {\"%s\": \"%s\"}\n", s1, s2, s1, s3);
    hashtable_insert(&hashtable, s1, strlen(s1)+1, s3, strlen(s3)+1);
*/	
	int i = 0;
    struct timespec t1;
    struct timespec t2;
	t1 = snap_time();
    for(i = 0; i < TEST_NUM; i++)
    {
		char key[KEY_SIZE]={0};
		char value[VALUE_SIZE] = {0};
		snprintf(key, KEY_SIZE, "test_key:%d", i);
		snprintf(value, VALUE_SIZE, "test_value:%d", i);

		printf("hashtable_insert key:%s  value:%s\n", key, value);
        hashtable_insert(&hashtable, key, sizeof(key), value, sizeof(value));
    }

    t2 = snap_time();
    fprintf(stderr, "Inserting %d keys (on preallocated table) took %.2f seconds\n", TEST_NUM, get_elapsed(t1, t2));
}


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

	do_task();

    pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

void dummy_task_3(void *arg) 
{
	fprintf(stderr, "%s  %d  %s\n", __FILE__, __LINE__, __FUNCTION__); 
    usleep(10000);
	
	do_task();
    
	pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

void dummy_task_4(void *arg) 
{
	fprintf(stderr, "%s  %d  %s\n", __FILE__, __LINE__, __FUNCTION__); 
	usleep(10000);
	
	do_task();
    
	pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

void dummy_task_5(void *arg) 
{
	fprintf(stderr, "%s  %d  %s\n", __FILE__, __LINE__, __FUNCTION__); 
    usleep(10000);
    
	do_task();
	
	pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
	/*
#ifndef  __WITH_MURMUR
	hashtable_init(&hashtable, HT_KEY_CONST | HT_VALUE_CONST, 0.05, NULL, NULL, NULL);
#else
    hashtable_init(&hashtable, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
#endif
    */
	
	hashtable_init(&hashtable, HT_KEY_CONST | HT_VALUE_CONST, 0.05);
    
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
    
	hashtable_destroy(&hashtable);

    return 0;
}
