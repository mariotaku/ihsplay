#include <assert.h>
#include "sample_data.h"
#include "sps_util.h"

int main() {
    sps_dimension_t dimension;
    assert(sps_util_parse_dimension_hevc(sample_data_sps_h265_1, sizeof(sample_data_sps_h265_1), &dimension));
    assert(dimension.width == 1920);
    assert(dimension.height == 1080);
    return 0;
}