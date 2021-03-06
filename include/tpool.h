#include <stddef.h>
#include <pthread.h>

#ifndef TPOOL_H
#define TPOOL_H

enum __future_flags {
    __FUTURE_RUNNING = 01,
    __FUTURE_FINISHED = 02,
    __FUTURE_TIMEOUT = 04,
    __FUTURE_CANCELLED = 010,
    __FUTURE_DESTROYED = 020,
};

typedef struct __threadtask {
    void *(*func)(void *);
    void *arg;
    struct __tpool_future *future;
    struct __threadtask *next;
} threadtask_t;

typedef struct __jobqueue {
    threadtask_t *head, *tail;
    pthread_cond_t cond_nonempty;
    pthread_mutex_t rwlock;
} jobqueue_t;

struct __tpool_future {
    int flag;
    void *result;
    pthread_mutex_t mutex;
    pthread_cond_t cond_finished;
};

struct __threadpool {
    size_t count;
    pthread_t *workers;
    jobqueue_t *jobqueue;
};

typedef struct __tpool_future *tpool_future_t;

typedef struct __threadpool *tpool_t;

/**
 * Create a thread pool containing specified number of threads.
 * If successful, the thread pool is returned. Otherwise, it
 * returns NULL.
 */
tpool_t tpool_create(size_t count);

/**
 * Schedules the specific function to be executed.
 * If successful, a future object representing the execution of
 * the task is returned. Otherwise, it returns NULL.
 */
tpool_future_t tpool_apply(tpool_t pool, void *(*func)(void *), void *arg);

/**
 * Wait for all pending tasks to complete before destroying the thread pool.
 */
int tpool_join(tpool_t pool);

/**
 * Return the result when it becomes available.
 * If @seconds is non-zero and the result does not arrive within specified time,
 * NULL is returned. Each tpool_future_get() resets the timeout status on
 * @future.
 */
void *tpool_future_get(tpool_future_t future, unsigned int seconds);

/**
 * Destroy the future object and free resources once it is no longer used.
 * It is an error to refer to a destroyed future object. Note that destroying
 * a future object does not prevent a pending task from being executed.
 */
int tpool_future_destroy(tpool_future_t future);

#endif
