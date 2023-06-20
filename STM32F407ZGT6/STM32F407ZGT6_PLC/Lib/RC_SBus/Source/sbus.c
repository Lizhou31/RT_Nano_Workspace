/*
 * sbus.c
 *
 *  Created on: Mar 13, 2023
 *      Author: paul
 */


#include "RC.h"
#include "sbus_datatype.h"

/**
 * @brief mapping channel output
 *
 */
#define CHANNEL_SCALE(raw_value)                                               		\
    ((raw_value * SBUS_TARGET_RANGE) / SBUS_RANGE_RANGE + SBUS_SCALE_OFFSET)

/**
 * @brief decode 88 bits (11 bytes) data to 8 channels (11 bits per chan)
 *
 * @param data  const raw packet data
 * @param nchannels decode channel counts
 * @param values decoded values
 */
static void decode_11_bits_channels(const uint8_t *data, uint8_t nchannels,
                                    uint16_t *values) {
    while (nchannels >= 8) {
        const struct Channels11Bit_8Chan *channels =
            (const struct Channels11Bit_8Chan *)data;
        values[0] = CHANNEL_SCALE(channels->ch0);
        values[1] = CHANNEL_SCALE(channels->ch1);
        values[2] = CHANNEL_SCALE(channels->ch2);
        values[3] = CHANNEL_SCALE(channels->ch3);
        values[4] = CHANNEL_SCALE(channels->ch4);
        values[5] = CHANNEL_SCALE(channels->ch5);
        values[6] = CHANNEL_SCALE(channels->ch6);
        values[7] = CHANNEL_SCALE(channels->ch7);

        nchannels -= 8;
        data += sizeof(*channels);
        values += 8;
    }
}

/**
 * @brief decode function
 *
 * @param handler
 */
static void decode(struct sbus_handler *handler) {

    /* decode data frame */
    decode_11_bits_channels((uint8_t *const) & (handler->rx_data.raw_data[1]),
                            16, handler->rx_data.channel_decoded);

    // test_decode_11_bits_channels((uint8_t *const) &
    //                                  (handler->rx_data.raw_data[1]),
    //                              16, handler->rx_data.channel_decoded);

    /* decode disconnect frame */
    if (handler->rx_data.raw_data[23] & (1 << 2))
        handler->ctrl.disconnected = 1;
    else
        handler->ctrl.disconnected = 0;
    handler->ctrl.decode_flag = 0;
}

/**
 * @brief chect connect status
 *
 * @param handler
 * @return uint8_t non-zero is disconnected, zero is connected
 */
static uint8_t is_disconnected(const struct sbus_handler *handler) {

    return handler->ctrl.disconnected;
}

/**
 * @brief register handler function
 *
 * @param handler
 */
void __sbus_op_register(struct sbus_handler *handler) {
    handler->sbus_decode = decode;
    handler->sbus_is_disconnected = is_disconnected;
    handler->ctrl.last_ms = handler->sbus_get_tick();
    handler->ctrl.disconnected = 1;
    handler->ctrl.decode_flag = 0;
    handler->rx_data.index = 0;
}
