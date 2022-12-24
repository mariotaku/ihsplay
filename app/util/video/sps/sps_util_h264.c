#include "sps_util.h"
#include "bitstream.h"
#include "common.h"

#define EXTENDED_SAR 255

static bool skip_vui_parameters(bitstream_t *buf);

static bool skip_hrd_parameters(bitstream_t *buf);

bool sps_util_parse_dimension_h264(const unsigned char *data, size_t size, sps_dimension_t *dimension) {
    int begin = 0;
    while ((begin = sps_util_nal_skip_start_code(data, size, begin)) >= 0) {
        if ((data[begin] & 0x1F) == 0x07/* SPS */) {
            break;
        }
    }
    if (begin < 0|| begin >= size) {
        return false;
    }
    bitstream_t buf;
    bitstream_init(&buf, data + begin, size - begin);

    uint8_t subwc[] = {1, 2, 2, 1};
    uint8_t subhc[] = {1, 2, 1, 1};

    uint32_t chroma_format_idc = 1;

    uint32_t width, height;

    // nalu_header
    bitstream_skip_bits_checked(&buf, 8);
    uint8_t profile_idc;
    bitstream_read8_checked(&buf, &profile_idc);
    // constraint_set[0-5]_flag
    bitstream_skip_bits_checked(&buf, 6);

    /* skip reserved_zero_2bits */
    if (!bitstream_skip_bits(&buf, 2))
        return false;

    // level_idc
    bitstream_skip_bits_checked(&buf, 8);

    uint32_t tmp;
    // id
    bitstream_read_ueg_checked(&buf, &tmp);

    if (profile_idc == 100 || profile_idc == 110 ||
        profile_idc == 122 || profile_idc == 244 || profile_idc == 44 ||
        profile_idc == 83 || profile_idc == 86 || profile_idc == 118 ||
        profile_idc == 128 || profile_idc == 138 || profile_idc == 139 ||
        profile_idc == 134 || profile_idc == 135) {
        if (!bitstream_read_ueg(&buf, &chroma_format_idc)) return false;
        if (chroma_format_idc > 3) return false;
        if (chroma_format_idc == 3) {
            // separate_colour_plane_flag
            bitstream_skip_bits_checked(&buf, 1);
        }

        // bit_depth_luma_minus8
        bitstream_read_ueg_checked(&buf, &tmp);
        // bit_depth_chroma_minus8
        bitstream_read_ueg_checked(&buf, &tmp);
        // qpprime_y_zero_transform_bypass_flag
        bitstream_skip_bits_checked(&buf, 1);

        bool scaling_matrix_present_flag;
        bitstream_read1_checked(&buf, &scaling_matrix_present_flag);
        if (scaling_matrix_present_flag) {
            for (int i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); i++) {
                bool scaling_list_present_flag;
                bitstream_read1_checked(&buf, &scaling_list_present_flag);
                if (scaling_list_present_flag) {
                    bitstream_skip_scaling_list_checked(&buf, i < 6 ? 16 : 64);
                }
            }
        }
    }

    // log2_max_frame_num_minus4
    bitstream_read_ueg_checked(&buf, &tmp);

    uint32_t pic_order_cnt_type;
    bitstream_read_ueg_checked(&buf, &pic_order_cnt_type);
    if (pic_order_cnt_type == 0) {
        // log2_max_pic_order_cnt_lsb_minus4
        bitstream_read_ueg_checked(&buf, &tmp);
    } else if (pic_order_cnt_type == 1) {
        // delta_pic_order_always_zero_flag
        bitstream_skip_bits_checked(&buf, 1);
        // offset_for_non_ref_pic
        bitstream_read_eg_checked(&buf, (int32_t *) (&tmp));
        // offset_for_top_to_bottom_field
        bitstream_read_eg_checked(&buf, (int32_t *) (&tmp));
        uint32_t num_ref_frames_in_pic_order_cnt_cycle;
        bitstream_read_ueg_checked(&buf, &num_ref_frames_in_pic_order_cnt_cycle);

        for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
            // offset_for_ref_frame[i]
            bitstream_read_eg_checked(&buf, (int32_t *) (&tmp));
        }
    }

    // num_ref_frames
    bitstream_read_ueg_checked(&buf, &tmp);
    // gaps_in_frame_num_value_allowed_flag
    bitstream_skip_bits_checked(&buf, 1);
    uint32_t pic_width_in_mbs_minus1;
    if (!bitstream_read_ueg(&buf, &pic_width_in_mbs_minus1)) return false;
    uint32_t pic_height_in_map_units_minus1;
    if (!bitstream_read_ueg(&buf, &pic_height_in_map_units_minus1)) return false;

    bool frame_mbs_only_flag = 0;
    bitstream_read1_checked(&buf, &frame_mbs_only_flag);

    if (!frame_mbs_only_flag) {
        // mb_adaptive_frame_field_flag
        bitstream_skip_bits_checked(&buf, 1);
    }

    // direct_8x8_inference_flag
    bitstream_skip_bits(&buf, 1);
    bool frame_cropping_flag;
    bitstream_read1_checked(&buf, &frame_cropping_flag);
    uint32_t frame_crop_left_offset = 0, frame_crop_right_offset = 0, frame_crop_top_offset = 0,
            frame_crop_bottom_offset = 0;
    if (frame_cropping_flag) {
        bitstream_read_ueg_checked(&buf, &frame_crop_left_offset);
        bitstream_read_ueg_checked(&buf, &frame_crop_right_offset);
        bitstream_read_ueg_checked(&buf, &frame_crop_top_offset);
        bitstream_read_ueg_checked(&buf, &frame_crop_bottom_offset);
    }

    bool vui_parameters_present_flag = false;
    bitstream_read1_checked(&buf, &vui_parameters_present_flag);
    if (vui_parameters_present_flag) {
        if (!skip_vui_parameters(&buf)) return false;
    }

    /* Calculate width and height */
    width = ((int) pic_width_in_mbs_minus1 + 1);
    width *= 16;
    height = ((int) pic_height_in_map_units_minus1 + 1);
    height *= 16 * (2 - frame_mbs_only_flag);

    if (frame_cropping_flag) {
        const uint32_t crop_unit_x = subwc[chroma_format_idc];
        const uint32_t crop_unit_y = subhc[chroma_format_idc] * (2 - frame_mbs_only_flag);

        width -= (frame_crop_left_offset + frame_crop_right_offset) * crop_unit_x;
        height -= (frame_crop_top_offset + frame_crop_bottom_offset) * crop_unit_y;
    }

    if ((int) width <= 0 || (int) height <= 0) {
        return false;
    }

    dimension->width = width;
    dimension->height = height;
    return true;
}

static bool skip_vui_parameters(bitstream_t *buf) {
    bool aspect_ratio_info_present_flag = false;
    bitstream_read1_checked(buf, &aspect_ratio_info_present_flag);
    if (aspect_ratio_info_present_flag) {
        uint8_t aspect_ratio_idc;
        bitstream_read8(buf, &aspect_ratio_idc);
        if (aspect_ratio_idc == EXTENDED_SAR) {
            // sar_width
            bitstream_skip_bits(buf, 16);
            // sar_height
            bitstream_skip_bits(buf, 16);
        }
    }

    bool overscan_info_present_flag = false;
    bitstream_read1_checked(buf, &overscan_info_present_flag);
    if (overscan_info_present_flag) {
        // overscan_appropriate_flag
        bitstream_skip_bits(buf, 1);
    }

    bool video_signal_type_present_flag = false;
    bitstream_read1_checked(buf, &video_signal_type_present_flag);
    if (video_signal_type_present_flag) {
        // video_format
        bitstream_skip_bits(buf, 3);
        // video_full_range_flag
        bitstream_skip_bits(buf, 1);
        bool colour_description_present_flag = false;
        bitstream_read1_checked(buf, &colour_description_present_flag);
        if (colour_description_present_flag) {
            bitstream_skip_bits(buf, 8);
            bitstream_skip_bits(buf, 8);
            bitstream_skip_bits(buf, 8);
        }
    }

    bool chroma_loc_info_present_flag = false;
    bitstream_read1_checked(buf, &chroma_loc_info_present_flag);
    if (chroma_loc_info_present_flag) {
        bitstream_skip_bits(buf, 5);
        bitstream_skip_bits(buf, 5);
    }

    bool timing_info_present_flag = false;
    bitstream_read1_checked(buf, &timing_info_present_flag);
    if (timing_info_present_flag) {
        // num_units_in_tick
        bitstream_skip_bits(buf, 32);
        // time_scale
        bitstream_skip_bits(buf, 32);
        // fixed_frame_rate_flag
        bitstream_skip_bits(buf, 1);
    }

    bool nal_hrd_parameters_present_flag = false;
    bitstream_read1_checked(buf, &nal_hrd_parameters_present_flag);
    if (nal_hrd_parameters_present_flag) {
        if (!skip_hrd_parameters(buf))
            return false;
    }

    bool vcl_hrd_parameters_present_flag = false;
    bitstream_read1_checked(buf, &vcl_hrd_parameters_present_flag);
    if (vcl_hrd_parameters_present_flag) {
        if (!skip_hrd_parameters(buf))
            return false;
    }

    if (nal_hrd_parameters_present_flag || vcl_hrd_parameters_present_flag) {
        // low_delay_hrd_flag
        bitstream_skip_bits(buf, 1);
    }

    // pic_struct_present_flag
    bitstream_skip_bits(buf, 1);
    bool bitstream_restriction_flag = false;
    bitstream_read1_checked(buf, &bitstream_restriction_flag);
    if (bitstream_restriction_flag) {
        // motion_vectors_over_pic_boundaries_flag
        bitstream_skip_bits(buf, 1);
        uint32_t tmp;
        // max_bytes_per_pic_denom
        bitstream_read_ueg(buf, &tmp);
        // max_bits_per_mb_denom
        bitstream_read_ueg(buf, &tmp);
        // log2_max_mv_length_horizontal
        bitstream_read_ueg(buf, &tmp);
        // log2_max_mv_length_vertical
        bitstream_read_ueg(buf, &tmp);
        // num_reorder_frames
        bitstream_read_ueg(buf, &tmp);
        // max_dec_frame_buffering
        bitstream_read_ueg(buf, &tmp);
    }

    return true;
}

static bool skip_hrd_parameters(bitstream_t *buf) {
    uint32_t cpb_cnt_minus1;
    bitstream_read_ueg(buf, &cpb_cnt_minus1);
    if (cpb_cnt_minus1 > 31) return false;

    // bit_rate_scale
    bitstream_skip_bits(buf, 4);
    // cpb_size_scale
    bitstream_skip_bits(buf, 4);

    uint32_t tmp;

    for (int sched_sel_idx = 0; sched_sel_idx <= cpb_cnt_minus1; sched_sel_idx++) {
        // bit_rate_value_minus1[sched_sel_idx]
        bitstream_read_ueg(buf, &tmp);
        // cpb_size_value_minus1[sched_sel_idx]
        bitstream_read_ueg(buf, &tmp);
        // cbr_flag[sched_sel_idx]
        bitstream_skip_bits(buf, 1);
    }

    // initial_cpb_removal_delay_length_minus1
    bitstream_skip_bits(buf, 5);
    // cpb_removal_delay_length_minus1
    bitstream_skip_bits(buf, 5);
    // dpb_output_delay_length_minus1
    bitstream_skip_bits(buf, 5);
    // time_offset_length
    bitstream_skip_bits(buf, 5);
    return true;
}
