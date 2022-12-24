
#include "bitstream.h"


void bitstream_init(bitstream_t *buf, const unsigned char *data, size_t size) {
    buf->data = data;
    buf->data_size = size;
    buf->offset = 0;
    buf->consecutive_zeroes = 0;
}

bool bitstream_read_bits(bitstream_t *buf, uint32_t size, uint32_t *value) {
    if (size > 32) return false;
    uint32_t result = 0;
    for (uint32_t i = 0; i < size; i++) {
        if ((buf->offset + i) % 8 == 0) {
            uint32_t byte_index = (buf->offset + i) / 8;
            if (byte_index >= buf->data_size) {
                return false;
            }
            unsigned char b = buf->data[byte_index];
            if (b == 0) {
                buf->consecutive_zeroes++;
            } else if (b == 0x03 && buf->consecutive_zeroes == 2) {
                if (++byte_index >= buf->data_size) {
                    return false;
                }
                buf->offset += 8;
                buf->consecutive_zeroes = buf->data[byte_index] == 0;
            } else {
                buf->consecutive_zeroes = 0;
            }
        }
        uint32_t cur_offset = buf->offset + i;
        uint32_t byte_index = cur_offset / 8;
        if (byte_index >= buf->data_size) {
            return false;
        }
        uint8_t bit_offset = 7 - cur_offset % 8;
        result |= (buf->data[byte_index] >> bit_offset & 0x1) << (size - i - 1);
    }
    buf->offset += size;
    *value = result;
    return true;
}


bool bitstream_skip_bits(bitstream_t *buf, uint32_t size) {
    uint32_t tmp;
    for (int i = 0; i < size; i += 16) {
        if (!bitstream_read_bits(buf, size - i < 16 ? size - i : 16, &tmp)) return false;
    }
    return true;
}

bool bitstream_read_ueg(bitstream_t *buf, uint32_t *value) {
    uint32_t bitcount = 0;
    uint32_t tmp;

    for (;;) {
        if (!bitstream_read_bits(buf, 1, &tmp)) return false;
        if (tmp == 0) {
            bitcount++;
        } else {
            // bitOffset--;
            break;
        }
    }

    // bitOffset --;
    uint32_t result = 0;
    if (bitcount) {
        if (!bitstream_read_bits(buf, bitcount, &tmp)) {
            return false;
        }
        result = (uint32_t) ((1 << bitcount) - 1 + tmp);
    }
    *value = result;
    return true;
}

bool bitstream_read_eg(bitstream_t *buf, int32_t *value) {
    uint32_t tmp;
    if (!bitstream_read_ueg(buf, &tmp)) {
        return false;
    }
    if (tmp & 0x01) {
        *value = (int32_t) (tmp + 1) / 2;
    } else {
        *value = (int32_t) -(tmp / 2);
    }
    return true;
}

bool bitstream_skip_scaling_list(bitstream_t *buf, uint8_t count) {
    uint32_t lastScale = 8, nextScale = 8;
    int32_t deltaScale;
    for (uint8_t j = 0; j < count; j++) {
        if (nextScale != 0) {
            if (!bitstream_read_eg(buf, &deltaScale)) return false;
            nextScale = (lastScale + deltaScale + 256) % 256;
        }
        lastScale = (nextScale == 0 ? lastScale : nextScale);
    }
    return true;
}
