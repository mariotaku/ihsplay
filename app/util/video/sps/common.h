#pragma once

#include <stddef.h>
#include <stdint.h>

#include "bitstream.h"

#define bitstream_skip_bits_checked(s, v) if (!bitstream_skip_bits((s), (v))) return false

#define bitstream_read1_checked(s, v) if (!bitstream_read1((s), (v))) return false

#define bitstream_read2_checked(s, v) if (!bitstream_read2((s), (v))) return false

#define bitstream_read3_checked(s, v) if (!bitstream_read3((s), (v))) return false

#define bitstream_read4_checked(s, v) if (!bitstream_read4((s), (v))) return false

#define bitstream_read8_checked(s, v) if (!bitstream_read8((s), (v))) return false

#define bitstream_read_eg_checked(s, v) if (!bitstream_read_eg((s), (v))) return false

#define bitstream_read_ueg_checked(s, v) if (!bitstream_read_ueg((s), (v))) return false

#define bitstream_skip_scaling_list_checked(s, v) if (!bitstream_skip_scaling_list((s), (v))) return false

#define CHECK_RETURN(ret) if (!(ret)) return false

/**
 *
 * @return Index of the first byte of the NAL needed
 */
int sps_util_nal_skip_start_code(const unsigned char *data, size_t size, size_t begin);