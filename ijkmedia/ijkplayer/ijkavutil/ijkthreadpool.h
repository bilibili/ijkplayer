/*
 * This file is part of ijkPlayer.
 *
 * ijkPlayer is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ijkPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have ijkPlayer a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _IJK_THREADPOOL_H_
#define _IJK_THREADPOOL_H_

#include <pthread.h>

#define MAX_THREADS 100
#define MAX_QUEUE 1024

typedef enum {
    IJK_THREADPOOL_INVALID        = -1,
    IJK_THREADPOOL_LOCK_FAILURE   = -2,
    IJK_THREADPOOL_QUEUE_FULL     = -3,
    IJK_THREADPOOL_SHUTDOWN       = -4,
    IJK_THREADPOOL_THREAD_FAILURE = -5
} IjkThreadPoolErrorType;

typedef enum {
    IJK_IMMEDIATE_SHUTDOWN = 1,
    IJK_LEISURELY_SHUTDOWN = 2
} IjkThreadPoolShutdownType;

typedef void (*Runable)(void *, void *);
/**
 *  @struct ThreadPoolTask
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var in_arg Argument to be passed to the function.
 *  @var out_arg Argument to be passed to the call function.
 */

typedef struct IjkThreadPoolTask {
    Runable function;
    void *in_arg;
    void *out_arg;
} IjkThreadPoolTask;

/**
 *  @struct ThreadPoolContext
 *  @brief The threadpool context struct
 *
 *  @var notify        Condition variable to notify worker threads.
 *  @var threads       Array containing worker threads ID.
 *  @var thread_count  Number of threads
 *  @var queue         Array containing the task queue.
 *  @var queue_size    Size of the task queue.
 *  @var queue_head    Index of the first element.
 *  @var queue_tail    Index of the next element.
 *  @var pending_count Number of pending tasks
 *  @var shutdown      Flag indicating if the pool is shutting down
 *  @var started       Number of started threads
 */
typedef struct IjkThreadPoolContext {
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    IjkThreadPoolTask *queue;
    int thread_count;
    int queue_size;
    int queue_head;
    int queue_tail;
    int pending_count;
    int shutdown;
    int started_count;
} IjkThreadPoolContext;

IjkThreadPoolContext *ijk_threadpool_create(int thread_count, int queue_size, int flags);

int ijk_threadpool_add(IjkThreadPoolContext *ctx, Runable function,
                   void *in_arg, void *out_arg, int flags);

int ijk_threadpool_destroy(IjkThreadPoolContext *ctx, int flags);

#endif /* _IJK_THREADPOOL_H_ */
