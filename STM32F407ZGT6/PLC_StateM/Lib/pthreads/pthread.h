/**
 * @file pthread.h
 * @author Lizhou
 * @brief a pthread features implementation test
 * @version 0.1
 * @date 2023-10-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#include <rtthread.h>
#include "libc_errno.h"

#define PTHREAD_COND_INITIALIZER \
    {                            \
        -1                       \
    }
#define PTHREAD_RWLOCK_INITIALIZER \
    {                              \
        -1                         \
    }
#define PTHREAD_MUTEX_INITIALIZER \
    {                             \
        -1                        \
    }

typedef long pthread_t;
typedef long pthread_condattr_t;
typedef long pthread_rwlockattr_t;
typedef long pthread_mutexattr_t;

enum
{
    PTHREAD_MUTEX_NORMAL = 0,
    PTHREAD_MUTEX_RECURSIVE = 1,
    PTHREAD_MUTEX_ERRORCHECK = 2,
    PTHREAD_MUTEX_ERRORCHECK_NP = PTHREAD_MUTEX_ERRORCHECK,
    PTHREAD_MUTEX_RECURSIVE_NP = PTHREAD_MUTEX_RECURSIVE,
    PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
};

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

struct pthread_mutex
{
    pthread_mutexattr_t attr;
    struct rt_mutex lock;
};
typedef struct pthread_mutex pthread_mutex_t;

/* pthread mutex interface */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
// trylock
// getprioceiling
// setprioceiling

// pthread_mutexattr

#endif /* _PTHREAD_H_ */