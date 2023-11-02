/**
 * @file base_test.c
 * @author Lizhou
 * @brief 
 * @version 0.1
 * @date 2023-10-17
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "unit_test.h"

void setUp(void) {}
void tearDown(void) {}

void test_unity(void)
{
    TEST_ASSERT_MESSAGE(1, "unity doesn't work :(");
}

#ifdef UNIT_TEST_ALL
void unit_test_all(void)
{
    UNITY_BEGIN();
    rt_kprintf("\nTest Unity framwork work.\n\n");
    RUN_TEST(test_unity);
    unity_pthread_mutex_test();
    unity_pthread_cond_test();
    unity_pthread_rwlock_test();
    UNITY_END();
}
MSH_CMD_EXPORT(unit_test_all, unit test all);
#endif