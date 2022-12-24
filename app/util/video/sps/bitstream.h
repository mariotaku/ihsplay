#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct bitstream_t {
    const unsigned char *data;
    size_t data_size;
    uint32_t offset;
    uint32_t consecutive_zeroes;
} bitstream_t;

void bitstream_init(bitstream_t *buf, const unsigned char *data, size_t size);

bool bitstream_read_bits(bitstream_t *buf, uint32_t size, uint32_t *value);

bool bitstream_read_eg(bitstream_t *buf, int32_t *value);

bool bitstream_read_ueg(bitstream_t *buf, uint32_t *value);

bool bitstream_skip_bits(bitstream_t *buf, uint32_t size);

bool bitstream_skip_scaling_list(bitstream_t *buf, uint8_t count);

static inline bool bitstream_read1(bitstream_t *buf, bool *value) {
    uint32_t tmp;
    if (!bitstream_read_bits(buf, 1, &tmp)) return false;
    *value = tmp;
    return true;
}

static inline bool bitstream_read2(bitstream_t *buf, uint8_t *value) {
    uint32_t tmp;
    if (!bitstream_read_bits(buf, 2, &tmp)) return false;
    *value = tmp;
    return true;
}

static inline bool bitstream_read3(bitstream_t *buf, uint8_t *value) {
    uint32_t tmp;
    if (!bitstream_read_bits(buf, 3, &tmp)) return false;
    *value = tmp;
    return true;
}

static inline bool bitstream_read4(bitstream_t *buf, uint8_t *value) {
    uint32_t tmp;
    if (!bitstream_read_bits(buf, 4, &tmp)) return false;
    *value = tmp;
    return true;
}

static inline bool bitstream_read8(bitstream_t *buf, uint8_t *value) {
    uint32_t tmp;
    if (!bitstream_read_bits(buf, 8, &tmp)) return false;
    *value = tmp;
    return true;
}
