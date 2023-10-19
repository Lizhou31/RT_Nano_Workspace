/**
 * @file test_pthread_mutex.c
 * @author Lizhou (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-10-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "unit_test.h"

void test_pthread_mutex_init_and_destroy(void)
{
    pthread_mutex_t p_mutex;
    pthread_mutex_init(&p_mutex, RT_NULL);

    TEST_ASSERT_NOT_NULL_MESSAGE(&p_mutex, "test mutex null failed");

    TEST_ASSERT_EQUAL_INT32_MESSAGE(RT_Object_Class_Mutex, p_mutex.lock.parent.parent.type, "check rt_mutex type correct");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&(p_mutex.lock.parent.parent.list), p_mutex.lock.parent.parent.list.next, "check rt_mutex not in object container 1");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(&(p_mutex.lock.parent.parent.list), p_mutex.lock.parent.parent.list.prev, "check rt_mutex not in object container 1");

    pthread_mutex_destroy(&p_mutex);
    uint8_t c = 0;
    TEST_ASSERT_EACH_EQUAL_MEMORY_MESSAGE(&c, &(p_mutex.lock), 1, sizeof(struct rt_mutex), "check mutex memory set 0 correct");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-1, p_mutex.attr, "check attr error");
}

static int counter = 0;
static int task_report[2] = {0};
/* Mutex test thread */
static void mutex_test_thread(void *param)
{
    pthread_mutex_t *p_mutex = (pthread_mutex_t *)param;
    pthread_mutex_lock(p_mutex);

    unsigned long i = 0;
    counter += 1;

    rt_thread_delay(100);
    task_report[counter - 1] = counter;

    pthread_mutex_unlock(p_mutex);
}

void test_mutex_work(void)
{
    pthread_mutex_t p_mutex;
    pthread_mutex_init(&p_mutex, RT_NULL);

    rt_thread_t mutex_test_thread_1 = rt_thread_create("task 1", mutex_test_thread, &p_mutex, 512U, 15, 10);
    rt_thread_t mutex_test_thread_2 = rt_thread_create("task 2", mutex_test_thread, &p_mutex, 512U, 15, 10);

    rt_thread_startup(mutex_test_thread_1);
    rt_thread_startup(mutex_test_thread_2);

    pthread_mutex_lock(&p_mutex);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, task_report[0], "Task 1 error.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, task_report[1], "Task 2 error.");
    pthread_mutex_unlock(&p_mutex);

    pthread_mutex_destroy(&p_mutex);
}

int unity_pthread_mutex_test(void)
{
    rt_kprintf("\nPthread Mutex Unit Test\n\n");
#ifndef UNITT_TEST_ALL
    UNITY_BEGIN();
#endif
    RUN_TEST(test_pthread_mutex_init_and_destroy);
    RUN_TEST(test_mutex_work);
#ifndef UNITT_TEST_ALL
    UNITY_END();
#endif
}
#ifndef UNITT_TEST_ALL
#ifdef MUTEX_UNITTEST
MSH_CMD_EXPORT(unity_pthread_mutex_test, pthread mutex unit test);
#endif
#endif
