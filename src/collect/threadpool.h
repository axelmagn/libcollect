#ifndef thread_Pool_h
#define thread_Pool_h

#include<pthread.h>
#include<collect/list.h>

/**
 * @brief a task for the thread pool
 */
typedef struct TPoolTask {
	void (* function)(void *);
	void *argument;
} TPoolTask;

/**
 * @brief a pthread pool
 */
typedef struct TPool {
	pthread_mutex_t lock;
	pthread_cond_t notify_add;
	List *threads;
	List *task_queue;
	int is_started;
	int shutdown_state;
} TPool;

TPool *TPool_create(int thread_count);
void TPool_add(TPool *pool, void (*function)(void *), void *args);
void TPool_shutdown(TPool *pool);
void TPool_destroy(TPool *pool);

#endif
