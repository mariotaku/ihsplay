/**
 * @see https://www.itu.int/rec/T-REC-H.265
 */
#include <stdio.h>
#include "sps_util.h"
#include "bitstream.h"
#include "common.h"

#define EXTENDED_SAR 255

static bool parse_profile_info(bitstream_t *buf);

static bool parse_profile_tier_level(bitstream_t *buf, uint8_t max_sub_layers_minus1);

bool sps_util_parse_dimension_hevc(const unsigned char *data, size_t size, sps_dimension_t *dimension) {
    int begin = 0;
    while ((begin = sps_util_nal_skip_start_code(data, size, begin)) >= 0) {
        if ((data[begin] & 0x7E) >> 1 == 33/* SPS */) {
            break;
        }
    }
    if (begin < 0 || begin >= size) {
        return false;
    }
    bitstream_t buf;
    bitstream_init(&buf, data + begin, size - begin);

    uint8_t subwc[] = {1, 2, 2, 1, 1};
    uint8_t subhc[] = {1, 2, 1, 1, 1};

    uint8_t max_sub_layers_minus1 = 0;

    uint32_t tmp;

    // sps_video_parameter_set_id
    bitstream_skip_bits_checked(&buf, 4);
    // sps_max_sub_layers_minus1
    bitstream_read3_checked(&buf, &max_sub_layers_minus1);
    // sps_temporal_id_nesting_flag
    bitstream_skip_bits(&buf, 1);

    if (!parse_profile_tier_level(&buf, max_sub_layers_minus1)) {
        return false;
    }

    // seq_parameter_set_id
    if (!bitstream_read_ueg(&buf, &tmp)) return false;

    uint32_t chroma_format_idc;
    if (!bitstream_read_ueg(&buf, &chroma_format_idc)) return false;
    if (chroma_format_idc == 3) {
        // separate_colour_plane_flag:1
        bitstream_skip_bits(&buf, 1);
    }

    uint32_t pic_width_in_luma_samples;
    uint32_t pic_height_in_luma_samples;
    if (!bitstream_read_ueg(&buf, &pic_width_in_luma_samples)) return false;
    if (!bitstream_read_ueg(&buf, &pic_height_in_luma_samples)) return false;

    if (pic_width_in_luma_samples <= 0 || pic_height_in_luma_samples <= 0) return false;

    bool conformance_window_flag = false;
    bitstream_read1(&buf, &conformance_window_flag);
    uint32_t conf_win_left_offset = 0, conf_win_right_offset = 0, conf_win_top_offset = 0, conf_win_bottom_offset = 0;
    if (conformance_window_flag) {
        bitstream_read_ueg_checked(&buf, &conf_win_left_offset);
        bitstream_read_ueg_checked(&buf, &conf_win_right_offset);
        bitstream_read_ueg_checked(&buf, &conf_win_top_offset);
        bitstream_read_ueg_checked(&buf, &conf_win_bottom_offset);
    }

    uint32_t width = pic_width_in_luma_samples, height = pic_height_in_luma_samples;
    if (conformance_window_flag) {
        const uint8_t crop_unit_x = subwc[chroma_format_idc];
        const uint8_t crop_unit_y = subhc[chroma_format_idc];
        width -= (conf_win_left_offset + conf_win_right_offset) * crop_unit_x;
        height -= (conf_win_top_offset + conf_win_bottom_offset) * crop_unit_y;
    }

    if ((int) width <= 0 || (int) height <= 0 || width % 2 != 0 || height % 2 != 0) {
        return false;
    }

    dimension->width = width;
    dimension->height = height;
    return true;
}

static bool parse_profile_info(bitstream_t *buf) {
    // profile_space:2
    bitstream_skip_bits_checked(buf, 2);
    // tier_flag:1
    bitstream_skip_bits_checked(buf, 1);
    // profile_idc:5
    bitstream_skip_bits_checked(buf, 5);

    for (int i = 0; i < 32; i++) {
        // profile_compatibility_flag[i]
        bitstream_skip_bits_checked(buf, 1);
    }
    // progressive_source_flag
    bitstream_skip_bits_checked(buf, 1);
    // interlaced_source_flag
    bitstream_skip_bits_checked(buf, 1);
    // non_packed_constraint_flag
    bitstream_skip_bits_checked(buf, 1);
    // frame_only_constraint_flag
    bitstream_skip_bits_checked(buf, 1);

    bitstream_skip_bits_checked(buf, 44);

    return true;
}

static bool parse_profile_tier_level(bitstream_t *buf, uint8_t max_sub_layers_minus1) {
    bool sub_layer_profile_present_flag[6];
    bool sub_layer_level_present_flag[6];

    CHECK_RETURN(parse_profile_info(buf));

    // level_idc
    bitstream_skip_bits(buf, 8);

    for (int i = 0; i < max_sub_layers_minus1; i++) {
        bitstream_read1_checked(buf, &sub_layer_profile_present_flag[i]);
        bitstream_read1_checked(buf, &sub_layer_level_present_flag[i]);
    }

    if (max_sub_layers_minus1 > 0) {
        for (int i = max_sub_layers_minus1; i < 8; i++) {
            // skip 2 bits
            bitstream_skip_bits_checked(buf, 2);
        }
    }

    for (int i = 0; i < max_sub_layers_minus1; i++) {
        if (sub_layer_profile_present_flag[i]) {
            CHECK_RETURN(parse_profile_info(buf));
        }

        if (sub_layer_level_present_flag[i]) {
            // sub_layer_level_idc[i]
            bitstream_skip_bits_checked(buf, 8);
        }
    }
    return true;
}