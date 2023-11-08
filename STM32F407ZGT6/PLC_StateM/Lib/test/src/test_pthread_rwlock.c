/**
 * @file test_pthread_rwlock.c
 * @author Lizhou
 * @brief
 * @version 0.1
 * @date 2023-11-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "unit_test.h"

void test_pthread_rwlock_init_and_destroy(void)
{
    pthread_rwlock_t p_rwlock;
    pthread_rwlock_init(&p_rwlock, RT_NULL);

    TEST_ASSERT_NOT_NULL_MESSAGE(&p_rwlock, "test rwlock not null failed");
    TEST_ASSERT_NOT_NULL_MESSAGE(&(p_rwlock.rw_mutex), "test rwlock.rw_mutex not null failed");
    TEST_ASSERT_NOT_NULL_MESSAGE(&(p_rwlock.rw_condreaders), "test rwlock.rw_condreaders not null failed");
    TEST_ASSERT_NOT_NULL_MESSAGE(&(p_rwlock.rw_condwriters), "test rwlock.rw_condwriters not null failed");

    TEST_ASSERT_EQUAL_INT32_MESSAGE(0, p_rwlock.rw_nwaitreaders, "test nwaitreaders zero failed");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(0, p_rwlock.rw_nwaitwriters, "test nwaitreaders zero failed");
    TEST_ASSERT_EQUAL_INT32_MESSAGE(0, p_rwlock.rw_refcount, "test rw_refcount zero failed");

    pthread_rwlock_destroy(&p_rwlock);
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, p_rwlock.attr, "check attr error");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, p_rwlock.rw_mutex.attr, "check mutex attr error");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, p_rwlock.rw_condreaders.attr, "check condreaders attr error");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, p_rwlock.rw_condwriters.attr, "check condwiters attr error");
}

static pthread_rwlock_t p_rwlock;
static uint8_t blocking_r = 0, blocking_w = 0;
static int result[2] = {0};

/* test pthread rwlock rdlock/wrlock */
static void test_pthread_rwlock_rdlock_thread(void *param)
{
    uint8_t *index = (uint8_t *)param;
    pthread_rwlock_rdlock(&p_rwlock);
    result[*index] = *index;
    pthread_rwlock_unlock(&p_rwlock);
    blocking_r = 1;
}
static void test_pthread_rwlock_wrlock_thread(void *param)
{
    uint8_t *index = (uint8_t *)param;
    pthread_rwlock_wrlock(&p_rwlock);
    *index = 0;
    *(index + 1) = 1;
    pthread_rwlock_unlock(&p_rwlock);
    blocking_w = 1;
}

void test_pthread_rwlock_rdlock(void)
{
    pthread_rwlock_init(&p_rwlock, RT_NULL);
    blocking_r = 0;
    blocking_w = 0;
    result[0] = 0;
    result[1] = 0;

    pthread_rwlock_wrlock(&p_rwlock);

    uint8_t read_memory[2] = {0};
    rt_thread_t read_thread_A = rt_thread_create("read_threadA",
                                                 test_pthread_rwlock_rdlock_thread,
                                                 &(read_memory[0]), 512U, 15, 10);
    rt_thread_t read_thread_B = rt_thread_create("read_threadB",
                                                 test_pthread_rwlock_rdlock_thread,
                                                 &(read_memory[1]), 512U, 15, 10);
    rt_thread_t write_thread = rt_thread_create("write_thread",
                                                test_pthread_rwlock_wrlock_thread,
                                                read_memory, 512U, 15, 10);

    rt_thread_startup(read_thread_A);
    rt_thread_startup(read_thread_B);
    rt_thread_startup(write_thread);

    rt_thread_mdelay(100);

    /* test read lock blocking */
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, blocking_r, "read lock is not blocking.");

    pthread_rwlock_unlock(&p_rwlock);

    rt_thread_mdelay(100);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result[0], "read thread A failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result[1], "read thread B failed.");

    pthread_rwlock_destroy(&p_rwlock);
}

void test_pthread_rwlock_wrlock(void)
{
    pthread_rwlock_init(&p_rwlock, RT_NULL);
    blocking_r = 0;
    blocking_w = 0;
    result[0] = 0;
    result[1] = 0;

    pthread_rwlock_rdlock(&p_rwlock);

    uint8_t write_memory[4] = {0};
    rt_thread_t write_thread_A = rt_thread_create("write_threadA",
                                                  test_pthread_rwlock_wrlock_thread,
                                                  write_memory, 512U, 15, 10);
    rt_thread_t write_thread_B = rt_thread_create("write_threadB",
                                                  test_pthread_rwlock_wrlock_thread,
                                                  &(write_memory[2]), 512U, 15, 10);
    rt_thread_startup(write_thread_A);
    rt_thread_startup(write_thread_B);

    rt_thread_mdelay(100);

    /* test read lock blocking */
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, blocking_w, "write lock is not blocking.");

    pthread_rwlock_unlock(&p_rwlock);

    rt_thread_mdelay(100);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, write_memory[0], "write thread A first element failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, write_memory[1], "write thread A second element failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, write_memory[2], "write thread B first element failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, write_memory[3], "write thread B second element failed.");

    pthread_rwlock_destroy(&p_rwlock);
}

static void test_pthread_rwlock_tryrdlock_thread(void *param)
{
    int *test = (int *)param;

    int ret = pthread_rwlock_tryrdlock(&p_rwlock);
    if (EBUSY == ret)
    {
        result[0] = ret;
        return;
    }
    else
        result[0] = *test;
    pthread_rwlock_unlock(&p_rwlock);
}
void test_pthread_rwlock_tryrdlock(void)
{
    pthread_rwlock_init(&p_rwlock, RT_NULL);
    result[0] = 0;

    pthread_rwlock_wrlock(&p_rwlock);

    int read_memory = 0;
    rt_thread_t read_thread_A = rt_thread_create("read_threadA",
                                                 test_pthread_rwlock_tryrdlock_thread,
                                                 &read_memory, 512U, 15, 10);

    rt_thread_startup(read_thread_A);

    /* test read lock return EBUSY */
    TEST_ASSERT_EQUAL_INT_MESSAGE(EBUSY, result[0], "read lock not return EBUSY.");

    read_memory = 1;
    pthread_rwlock_unlock(&p_rwlock);
    rt_thread_t read_thread_B = rt_thread_create("read_threadB",
                                                 test_pthread_rwlock_tryrdlock_thread,
                                                 &read_memory, 512U, 15, 10);

    rt_thread_startup(read_thread_B);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result[0], "read thread B second failed.");

    pthread_rwlock_destroy(&p_rwlock);
}

static void test_pthread_rwlock_trywrlock_thread(void *param)
{
    uint8_t *test = (uint8_t *)param;
    int ret = pthread_rwlock_trywrlock(&p_rwlock);
    if (0 != ret)
    {
        result[0] = ret;
        return;
    }
    else if (0 == ret)
    {
        result[0] = *test;
    }
    pthread_rwlock_unlock(&p_rwlock);
}
void test_pthread_rwlock_trywrlock(void)
{
    pthread_rwlock_init(&p_rwlock, RT_NULL);
    result[0] = 0;

    pthread_rwlock_wrlock(&p_rwlock);

    int write_memory = 0;
    rt_thread_t write_thread_A = rt_thread_create("write_threadA",
                                                  test_pthread_rwlock_trywrlock_thread,
                                                  &write_memory, 512U, 15, 10);

    rt_thread_startup(write_thread_A);

    /* test write lock return EBUSY */
    TEST_ASSERT_EQUAL_INT_MESSAGE(EBUSY, result[0], "read lock not return EBUSY.");

    write_memory = 1;
    pthread_rwlock_unlock(&p_rwlock);

    rt_thread_t write_thread_B = rt_thread_create("write_threadB",
                                                 test_pthread_rwlock_trywrlock_thread,
                                                 &write_memory, 512U, 15, 10);

    rt_thread_startup(write_thread_B);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, result[0], "write thread B second failed.");

    pthread_rwlock_destroy(&p_rwlock); 
}

int unity_pthread_rwlock_test(void)
{
    rt_kprintf("\nPthread RWlock Unit Test\n\n");
#ifndef UNIT_TEST_ALL
    UNITY_BEGIN();
#endif
    RUN_TEST(test_pthread_rwlock_init_and_destroy);
    RUN_TEST(test_pthread_rwlock_rdlock);
    RUN_TEST(test_pthread_rwlock_wrlock);
    RUN_TEST(test_pthread_rwlock_tryrdlock);
    RUN_TEST(test_pthread_rwlock_trywrlock);
#ifndef UNIT_TEST_ALL
    UNITY_END();
#endif
}
#ifndef UNIT_TEST_ALL
#ifdef MUTEX_UNITTEST
MSH_CMD_EXPORT(unity_pthread_rwlock_test, pthread rwlock unit test);
#endif
#endif
