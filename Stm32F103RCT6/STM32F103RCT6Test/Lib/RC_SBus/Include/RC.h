/*
 * RC.h
 *
 *  Created on: Mar 13, 2023
 *      Author: paul
 */

#ifndef INCLUDE_RC_H_
#define INCLUDE_RC_H_

#include "sbus_datatype.h"

/**
 * @brief define range mapping here, -+100% -> 1000..2000
 *
 */
#define SBUS_TARGET_MIN 0
#define SBUS_TARGET_MAX 255
#define SBUS_TARGET_RANGE (SBUS_TARGET_MAX - SBUS_TARGET_MIN)

/**
 * @brief define futaba sbus range 100% 352~1696
 * @brief define skydroid subus range 100% 282~1722
 */
#define SBUS_RANGE_MIN 282
#define SBUS_RANGE_MAX 1722
#define SBUS_RANGE_RANGE (SBUS_RANGE_MAX - SBUS_RANGE_MIN)

/**
 * @brief register sbus handler operation
 *
 */
void __sbus_op_register(struct sbus_handler*);

/**
 * @brief Caculate mapping offset
 *
 */
#define SBUS_SCALE_OFFSET                                                      \
    (SBUS_TARGET_MIN -                                                         \
     ((SBUS_TARGET_RANGE * SBUS_RANGE_MIN / SBUS_RANGE_RANGE)))

/**
 * @brief A tricky macro from linux kernel to detect type compatible.
 *
 */
#define BUILD_BUG_ON_ZERO(e) (sizeof(struct { int : -!!(e); }))

#define SBUS_DEFINE(name, SBUS_HANDLER)                                        \
    do {                                                                       \
    	SBUS_Handler *a;                                                       \
        BUILD_BUG_ON_ZERO(                                                     \
            !__builtin_types_compatible_p(typeof(SBUS_HANDLER), typeof(a)));   \
        __##name##_op_register(SBUS_HANDLER);                                  \
    } while (0)

#endif /* INCLUDE_RC_H_ */
