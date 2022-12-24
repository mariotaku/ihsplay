#include "common.h"

int sps_util_nal_skip_start_code(const unsigned char *data, size_t size, size_t begin) {
    int consecutive_zeroes = 0;
    bool found = false;
    for (size_t i = begin; i < size; ++i) {
        unsigned char b = data[i];
        if (found) {
            return i;
        } else if (b == 0) {
            consecutive_zeroes++;
        } else if (consecutive_zeroes >= 2 && b == 1) {
            found = true;
            continue;
        } else {
            consecutive_zeroes = 0;
        }
    }
    return -1;
}