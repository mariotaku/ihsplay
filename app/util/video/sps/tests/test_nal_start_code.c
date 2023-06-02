#include <assert.h>
#include "common.h"

int main() {
    unsigned char data1[] = {0x00, 0x00, 0x00, 0x01, 0x33};
    assert(sps_util_nal_skip_start_code(data1, 5, 0) == 4);
    unsigned char data2[] = {0x00, 0x00, 0x00, 0x02, 0x33};
    assert(sps_util_nal_skip_start_code(data2, 5, 0) == -1);
    unsigned char data3[] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x33};
    assert(sps_util_nal_skip_start_code(data3, 6, 0) == 5);
    unsigned char data4[] = {0x00, 0x01};
    assert(sps_util_nal_skip_start_code(data4, 2, 0) == -1);
    return 0;
}