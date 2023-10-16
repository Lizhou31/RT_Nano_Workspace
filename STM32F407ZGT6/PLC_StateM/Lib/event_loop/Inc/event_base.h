/**
 * @file event_base.h
 * @author Lizhou 
 * @brief 
 * @version 0.1
 * @date 2023-10-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/**
 * @brief 
 * 
 */
#define SUBSCRIPTION_MAX 8


struct event_base{
    int event_subscription[SUBSCRIPTION_MAX];
    int event_publisher;

    void *read_write_lock;

    int (*do_func)(void *param);

    void *data_pointer;
};