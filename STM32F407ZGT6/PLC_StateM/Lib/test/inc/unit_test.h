/**
 * @file test_pthread_mutex.h
 * @author Lizhou
 * @brief 
 * @version 0.1
 * @date 2023-10-16
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef __UNIT_TEST_H_
#define __UNIT_TEST_H_

#include "unity.h"
#include "main.h"
#include "pthread.h"

void test_unity(void);

#ifdef UNIT_TEST_ALL
int unity_pthread_mutex_test(void);
int unity_pthread_cond_test(void);
int unity_pthread_rwlock_test(void);
#endif
#endif /* __TEST_PTHREAD_MUTEX_H_ */