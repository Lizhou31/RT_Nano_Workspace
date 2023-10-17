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
#ifndef __TEST_PTHREAD_MUTEX_H_
#define __TEST_PTHREAD_MUTEX_H_

#include "unity.h"
#include "main.h"
#include "pthread.h"

int mutex_test(void);

#ifdef MUTEX_UNITTEST
MSH_CMD_EXPORT(mutex_test, mutex unit test);
#endif
#endif /* __TEST_PTHREAD_MUTEX_H_ */