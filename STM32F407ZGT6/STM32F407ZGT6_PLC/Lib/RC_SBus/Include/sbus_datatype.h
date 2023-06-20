/*
 * sbus_datatype.h
 *
 *  Created on: Mar 13, 2023
 *      Author: paul
 */

#ifndef INCLUDE_SBUS_DATATYPE_H_
#define INCLUDE_SBUS_DATATYPE_H_

#include "stdint.h"

typedef struct sbus_handler SBUS_Handler;

/**
 * @brief packed struct to decode 11 bit channel
 * @detailed 11 bits * 8 channel == 88 bits = 11 bytes,
 * 			 for s-bus 11 bits * 16 channel = 196 bits = 22 bytes.
 */
#pragma pack(push, r1)
#pragma pack(1)
struct Channels11Bit_8Chan{
	uint32_t ch0 : 11;
	uint32_t ch1 : 11;
	uint32_t ch2 : 11;
	uint32_t ch3 : 11;
	uint32_t ch4 : 11;
	uint32_t ch5 : 11;
	uint32_t ch6 : 11;
	uint32_t ch7 : 11;
};
#pragma pack(pop, r1)

/**
 * @brief s-bus control parameter / data
 *
 */
struct sbus_control {
    uint8_t decode_flag;  /*!< decode flag, set when decode process complete*/
    uint8_t disconnected; /*!< disconnected flag, set if radio controller
                             disconnected */
    volatile uint32_t last_ms;     /*!< to check last byte received time */
};

/**
 * @brief s-bus rx data
 *
 */
struct sbus_data {
    uint8_t index;                /*!< index to store raw bytes */
    uint8_t raw_data[25];         /*!< sbus raw data */
    uint16_t channel_decoded[16]; /*!< sbus decoded data */
};

/**
 * @brief s-bus handler
 * @details High level api, all the control methods/data are in here.
 *
 */
struct sbus_handler {
    struct sbus_data rx_data; /*!< sbus rx data struct*/
    struct sbus_control ctrl; /*!< sbus control struct */
    void (*sbus_getbyte)(SBUS_Handler *,
                         uint8_t);       /*!< sbus get byte function pointer */
    void (*sbus_raw_fetch)(SBUS_Handler *);	 /*!< sbus fetch data */
    void (*sbus_decode)(SBUS_Handler *); /*!< sbus decode function
                                                   pointer */
    uint8_t (*sbus_is_disconnected)(const SBUS_Handler *); /*!< sbus check
                                                            * connect status
                                                            * function pointer
                                                            */
    uint32_t (*sbus_get_tick)(void); /*! get system tick */
};


#endif /* INCLUDE_SBUS_DATATYPE_H_ */
