/**
 * @file pthread_rwlock.c
 * @author Lizhou
 * @brief 
 * @version 0.1
 * @date 2023-10-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "pthread.h"

const pthread_rwlockattr_t pthread_default_rwlockattr = PTHREAD_PROCESS_PRIVATE;

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
    if (!attr)
        return EINVAL;
    *attr = pthread_default_rwlockattr;

    return 0;
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
    if (!attr)
        return EINVAL;

    return 0;
}

int pthread_rwlock_init(pthread_rwlock_t *rwlock,
                        const pthread_rwlockattr_t *attr)
{
    if (!rwlock)
        return EINVAL;

    rwlock->attr = pthread_default_rwlockattr;
    pthread_mutex_init(&(rwlock->rw_mutex), NULL);
    pthread_cond_init(&(rwlock->rw_condreaders), NULL);
    pthread_cond_init(&(rwlock->rw_condwriters), NULL);

    rwlock->rw_nwaitwriters = 0;
    rwlock->rw_nwaitreaders = 0;
    rwlock->rw_refcount = 0;

    return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
        return EINVAL;
    if (rwlock->attr == -1)
        return 0; /* rwlock is not initialozed */

    if ((result = pthread_mutex_lock(&rwlock->rw_mutex)) != 0)
        return result;

    if (rwlock->rw_refcount != 0 ||
        rwlock->rw_nwaitwriters != 0 ||
        rwlock->rw_nwaitreaders != 0)
    {
        result = EBUSY;
        return result;
    }
    else
    {
        /* check whether busy */
        if (rt_list_isempty(&(rwlock->rw_condreaders.sem.parent.suspend_thread)))
        {
            if (rt_list_isempty(&(rwlock->rw_condreaders.sem.parent.suspend_thread)))
            {
                pthread_cond_destroy(&rwlock->rw_condreaders);
                pthread_cond_destroy(&rwlock->rw_condwriters);
            }
            else
            {
                result = EBUSY;
            }
        }
        else
            result = EBUSY;
    }

    result = pthread_mutex_unlock(&rwlock->rw_mutex);
    if (result == 0)
        pthread_mutex_destroy(&rwlock->rw_mutex);

    rwlock->attr = -1;
    return result;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
        return EINVAL;
    if (rwlock->attr == -1)
        pthread_rwlock_init(rwlock, NULL);

    if ((result = pthread_mutex_lock(&rwlock->rw_mutex)) != 0)
        return result;

    /* give preference to waiting writers */
    while (rwlock->rw_refcount < 0 || rwlock->rw_nwaitwriters > 0)
    {
        rwlock->rw_nwaitreaders++;
        /* rw_mutex will be released when waiting for rw_condreaders */
        result = pthread_cond_wait(&rwlock->rw_condreaders, &rwlock->rw_mutex);
        /* rw_mutex should have been taken again when returned from waiting */
        rwlock->rw_nwaitreaders--;
        if (result != 0) /* wait error */
            break;
    }

    /* another reader has a read lock */
    if (result == 0)
        rwlock->rw_refcount++;

    pthread_mutex_unlock(&rwlock->rw_mutex);

    return result;
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
    return 0;
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock, const struct timespec *abstime)
{
    return 0;
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock, const struct timespec *abstime)
{
    return 0;
}

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
    {
        return EINVAL;
    }
    if (rwlock->attr == -1)
    {
        pthread_rwlock_init(rwlock, RT_NULL);
    }

    if ((result = pthread_mutex_lock(&rwlock->rw_mutex)) != 0)
    {
        return result;
    }

    if (rwlock->rw_refcount > 0)
        rwlock->rw_refcount--; /* releasing a reader */
    else if (rwlock->rw_refcount == -1)
        rwlock->rw_refcount = 0; /* releasing a writer */

    /* give preference to waiting writers over wating readers */
    if (rwlock->rw_nwaitwriters > 0)
    {
        if (rwlock->rw_refcount == 0)
            result = pthread_cond_signal(&rwlock->rw_condwriters);
    }
    else if (rwlock->rw_nwaitreaders > 0)
    {
        result = pthread_cond_broadcast(&rwlock->rw_condreaders);
    }

    pthread_mutex_unlock(&(rwlock->rw_mutex));

    return result;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
        return EINVAL;
    if (rwlock->attr == -1)
        pthread_rwlock_init(rwlock, NULL);

    if ((result = pthread_mutex_lock(&rwlock->rw_mutex)) != 0)
        return result;

    while (rwlock->rw_refcount != 0)
    {
        rwlock->rw_nwaitwriters++;
        /* rw_mutex will be released when waiting for rw_condwriters */
        result = pthread_cond_wait(&rwlock->rw_condwriters, &rwlock->rw_mutex);
        /* rw_mutex should have been taken again when returned from waiting */
        rwlock->rw_nwaitwriters--;

        if (result != 0)
            break;
    }

    if (result == 0)
        rwlock->rw_refcount = -1;

    pthread_mutex_unlock(&rwlock->rw_mutex);

    return (result);
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
    return 0;
}