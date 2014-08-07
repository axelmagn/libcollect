#include <stdlib.h>
#include <pthread.h>

#include <dbg.h>
#include <collect/threadpool.h>

typedef enum {
	TPool_not_shutdown = 0,
	TPool_immediate_shutdown = 1,
	TPool_scheduled_shutdown = 2
} TPool_shutdown_state;

static void *TPool_consume(void *pool);

int TPool_free(void *pool);

TPool *TPool_create(int thread_count) 
{
	TPool *pool = malloc(sizeof(TPool));
	check(pool != NULL, "Failed to allocate TPool");

	// Initialize
	pool->is_started = 0;
	pool->shutdown_state = TPool_not_shutdown;

	// Create thread and task lists
	pool->threads = List_create();
	pool->task_queue = List_create();

	// Set up threading vars
	int err;
	err = pthread_mutex_init(&(pool->lock), NULL);
	check(err == 0, "Failed to initialize TPool->lock with error %d", err);
	err = pthread_cond_init(&(pool->notify_add), NULL);
	check(err == 0, "Failed to initialize TPool->notify_add with error %d", 
			err);

	// Start workers
	int i;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	for(i = 0; i < thread_count; i++) {
		pthread_t *thread = malloc(sizeof(pthread_t));
		List_push(pool->threads, (void *)thread);
		err = pthread_create(thread, &attr, TPool_consume, 
				(void *)pool);
		check(err == 0, "Failed to initialize thread %d with error %d",
				i, err);
	}

	return pool;
error:
	if(pool) { TPool_destroy(pool); }
	return NULL;
}


void TPool_add(TPool *pool, void (*function)(void *), void *args) 
{
	// TODO
}

void TPool_destroy(TPool *pool) 
{
	// TODO
}

static void *TPool_consume(void *pool) {
	// TODO
	return NULL;
};
