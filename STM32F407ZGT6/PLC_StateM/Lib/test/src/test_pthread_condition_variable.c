/**
 * @file test_pthread_condition_variable.c
 * @author Lizhou
 * @brief 
 * @version 0.1
 * @date 2023-10-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "unit_test.h"

/* test pthread cond initialize and destroy */
void test_pthread_cond_init_and_destroy(void)
{
    pthread_cond_t p_cond;
    pthread_cond_init(&p_cond, RT_NULL);

    TEST_ASSERT_NOT_NULL_MESSAGE(&p_cond, "test cond null failed");

    TEST_ASSERT_EQUAL_INT32_MESSAGE(RT_Object_Class_Semaphore, p_cond.sem.parent.parent.type, "check rt_sem type correct");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&(p_cond.sem.parent.parent.list), p_cond.sem.parent.parent.list.next, "check rt_sem not in object container 1");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&(p_cond.sem.parent.parent.list), p_cond.sem.parent.parent.list.prev, "check rt_sem not in object container 1");

    pthread_cond_destroy(&p_cond);
    uint8_t c = 0;
    TEST_ASSERT_EACH_EQUAL_MEMORY_MESSAGE(&c, &(p_cond.sem), 1, sizeof(struct rt_semaphore), "check rt_sem memory set 0 correct");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, p_cond.attr, "check attr error");
}

/* test pthread cond broadcast */
/* The pthread_cond_broadcast() function shall unblock
 * all threads currently blocked on the specified condition variable cond. 
 */
static pthread_mutex_t p_mutex;
static pthread_cond_t p_cond;
static int task_report[4] = {0};
static void test_ptherad_cond_broadcast_thread(void *param)
{
    int *task_report = (int *)param;
    pthread_mutex_lock(&p_mutex);
    if (task_report[0] == 0)
    {
        pthread_cond_wait(&p_cond, &p_mutex);
    }
    *task_report = 1;
    pthread_mutex_unlock(&p_mutex);
}
static void test_ptherad_cond_broadcast_main_thread(void *param)
{
    pthread_mutex_lock(&p_mutex);
    task_report[0] = 1;
    pthread_mutex_unlock(&p_mutex);

    pthread_cond_broadcast(&p_cond);
}
void test_ptherad_cond_broadcast(void)
{
    pthread_mutex_init(&p_mutex, RT_NULL);
    pthread_cond_init(&p_cond, RT_NULL);
    for (int i = 0; i < 4; i++)
    {
        task_report[i] = 0;
    }

    rt_thread_t cond_test_thread_main = rt_thread_create("task main",
                                                         test_ptherad_cond_broadcast_main_thread,
                                                         RT_NULL, 512U, 21, 10);
    rt_thread_t cond_test_thread_high = rt_thread_create("task high",
                                                         test_ptherad_cond_broadcast_thread,
                                                         &(task_report[1]), 512U, 15, 10);
    rt_thread_t cond_test_thread_medium = rt_thread_create("task medium",
                                                           test_ptherad_cond_broadcast_thread,
                                                           &(task_report[2]), 512U, 21, 10);
    rt_thread_t cond_test_thread_low = rt_thread_create("task low",
                                                        test_ptherad_cond_broadcast_thread,
                                                        &(task_report[3]), 512U, 24, 10);

    rt_thread_startup(cond_test_thread_low);
    rt_thread_mdelay(10);
    rt_thread_startup(cond_test_thread_medium);
    rt_thread_mdelay(10);
    rt_thread_startup(cond_test_thread_high);
    rt_thread_mdelay(10);
    rt_thread_startup(cond_test_thread_main);

    rt_thread_mdelay(100);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, task_report[1], "high priority thread error.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, task_report[2], "medium priority thread error.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, task_report[3], "low priority thread error.");

    pthread_mutex_destroy(&p_mutex);
    pthread_cond_destroy(&p_cond);
}

/* test pthread cond signal */
/* The pthread_cond_signal() function shall 
 * unblock at least one of the threads that are blocked 
 * on the specified condition variable cond (if any threads are blocked on cond).
*/
/* test pthread cond wait */
static void test_pthread_cond_signal_thread(void *param)
{
    pthread_mutex_lock(&p_mutex);
    if (task_report[0] == 0)
    {
        pthread_cond_wait(&p_cond, &p_mutex);
    }
    task_report[1] = 1;
    pthread_mutex_unlock(&p_mutex);
}
void test_pthread_cond_signal(void)
{
    int result = 0;
    result = pthread_mutex_init(&p_mutex, RT_NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "pthread mutex initialized failed.");
    result = pthread_cond_init(&p_cond, RT_NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "pthread cond initialized failed.");

    for (int i = 0; i < 4; i++)
    {
        task_report[i] = 0;
    }

    rt_thread_t cond_test_thread = rt_thread_create("task wait",
                                                    test_pthread_cond_signal_thread,
                                                    RT_NULL, 512U, 15, 10);
    rt_thread_startup(cond_test_thread);

    rt_thread_mdelay(10);
    result = pthread_mutex_lock(&p_mutex);
    task_report[0] = 1;

    pthread_mutex_unlock(&p_mutex);
    pthread_cond_signal(&p_cond);

    rt_thread_mdelay(10);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, task_report[1], "cond signal error.");

    pthread_mutex_destroy(&p_mutex);
    pthread_cond_destroy(&p_cond);
}

/* test pthread cond wait */
static void test_pthread_cond_wait_thread(void *param)
{
    pthread_mutex_lock(&p_mutex);
    if (task_report[0] == 0)
    {
        pthread_cond_wait(&p_cond, &p_mutex);
    }
    task_report[1] = 1;
    pthread_mutex_unlock(&p_mutex);
}
void test_pthread_cond_wait(void)
{
    int result = 0;
    result = pthread_mutex_init(&p_mutex, RT_NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "pthread mutex initialized failed.");
    result = pthread_cond_init(&p_cond, RT_NULL);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "pthread cond initialized failed.");

    for (int i = 0; i < 4; i++)
    {
        task_report[i] = 0;
    }

    rt_thread_t cond_test_thread = rt_thread_create("task wait",
                                                    test_pthread_cond_wait_thread,
                                                    RT_NULL, 512U, 15, 10);
    rt_thread_startup(cond_test_thread);

    rt_thread_mdelay(10);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, task_report[1], "cond wait error.");

    result = pthread_mutex_lock(&p_mutex);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "mutex lock failed.");

    task_report[0] = 1;
    pthread_mutex_unlock(&p_mutex);

    pthread_cond_broadcast(&p_cond);

    rt_thread_mdelay(10);

    pthread_mutex_destroy(&p_mutex);
    pthread_cond_destroy(&p_cond);
}

void test_pthread_cond_timewait(void)
{
}

int unity_pthread_cond_test(void)
{
    rt_kprintf("\nPthread Condition Variable Unit Test\n\n");
#ifndef UNIT_TEST_ALL
    UNITY_BEGIN();
#endif
    RUN_TEST(test_pthread_cond_init_and_destroy);
    RUN_TEST(test_ptherad_cond_broadcast);
    RUN_TEST(test_pthread_cond_signal);
    RUN_TEST(test_pthread_cond_wait);
#ifndef UNIT_TEST_ALL
    UNITY_END();
#endif
}

#ifndef UNIT_TEST_ALL
#ifdef CONDITION_VARIABLE_UNITTEST
MSH_CMD_EXPORT(unity_pthread_cond_test, pthread condition variable unit test);
#endif
#endif