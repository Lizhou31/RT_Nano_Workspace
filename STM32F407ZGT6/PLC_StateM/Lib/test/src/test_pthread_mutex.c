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

void setUp(void) {}
void tearDown(void) {}

void test_unity(void);
void test_unity(void)
{
    TEST_ASSERT(1);
}

int mutex_test(void)
{
    rt_kprintf("Hello\n");
    UNITY_BEGIN();
    RUN_TEST(test_unity);
    UNITY_END();
}

