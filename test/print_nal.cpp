//
// Created by zu on 2025/12/16.
//

#include "print_nal.h"
#include "Log.h"
#include "FileByteReader.h"
#include "H264/h264_analyze.h"

#define TAG "print_nal"

void print_nal(const char *path) {
    FileByteReader reader(path);
    H264_NAL *nal = nullptr;
    while ((nal = h264_parse_nal(reader)) != nullptr) {
        LOGD(TAG, "======================");
        LOGD(TAG, "nal_unit_type = %d, nal_ref_idc = %d", nal->nal_unit_type, nal->nal_ref_idc);
        if (nal->nal_unit_type == 7) {
            LOGD(TAG, "find sps unit, sps size = %d", nal->rbsp.size());
            H264_SPS *sps = h264_parse_sps(nal);
            if (sps != nullptr) {
                LOGD(TAG, "seq_parameter_set_id = %d, profile_idc = %d", sps->seq_parameter_set_id, sps->profile_idc);
                int width = (sps->pic_width_in_mbs_minus1 + 1) * 16;
                int height = (2 - sps->frame_mbs_only_flag) * (sps->pic_height_in_map_units_minus1 + 1) * 16;
                if (sps->frame_cropping_flag) {
                    int crop_unit_x = 0;
                    int crop_unit_y = 0;
                    if (h264_is_high_profile(sps->profile_idc)) {
                        // chroma_format_idc == 0
                        crop_unit_x = 1;
                        crop_unit_y = 2 - sps->frame_mbs_only_flag;
                        if (sps->chroma_format_idc == 1) {    // 4:2:0
                            crop_unit_x = 2;
                            crop_unit_y = 2 * (2 - sps->frame_mbs_only_flag);
                        } else if (sps->chroma_format_idc == 2) { // 4:2:2
                            crop_unit_x = 2;
                            crop_unit_y = 2 - sps->frame_mbs_only_flag;
                        } else if (sps->chroma_format_idc == 3) { // 4:4:4
                            crop_unit_x = 1;
                            crop_unit_y = 2 - sps->frame_mbs_only_flag;
                        }
                    } else {
                        // 按(4:2:0)计算，标准规定
                        crop_unit_x = 2;
                        crop_unit_y = 2 * (2 - sps->frame_mbs_only_flag);
                    }

                    LOGD(TAG, "chroma_format_idc = %d, frame_mbs_only_flag = %d, cropUnitX = %d, cropUnitY = %d", sps->chroma_format_idc, sps->frame_mbs_only_flag, crop_unit_x, crop_unit_y);
                    width -= (sps->frame_crop_left_offset + sps->frame_crop_right_offset) * crop_unit_x;
                    height -= (sps->frame_crop_top_offset + sps->frame_crop_bottom_offset) * crop_unit_y;
                }
                LOGD(TAG, "width = %d, height = %d", width, height);
                if (sps->vui != nullptr) {
                    H264_VUI *vui = sps->vui;
                    if (vui->timing_info_present_flag) {
                        LOGD(TAG, "vui.time_scale = %d", vui->time_scale);
                        LOGD(TAG, "vui.num_units_in_tick = %d", vui->num_units_in_tick);
                        LOGD(TAG, "fps = %f", vui->time_scale * 1.0f / (2 * vui->num_units_in_tick));
                    }
                }
                delete sps;
            } else {
                LOGE(TAG, "failed to parse sps");
            }

        } else if(nal->nal_unit_type == 8) {
            LOGD(TAG, "find pps unit, pps size = %d", nal->rbsp.size());
            H264_PPS *pps = h264_parse_pps(nal);
            if (pps != nullptr) {
                LOGD(TAG, "seq_parameter_set_id = %d", pps->seq_parameter_set_id);
                LOGD(TAG, "num_slic_groups_minus1 = %d", pps->num_slice_groups_minus1);
                LOGD(TAG, "pic_init_qp_minus26 = %d", pps->pic_init_qp_minus26);
                LOGD(TAG, "pic_init_qs_minus26 = %d", pps->pic_init_qs_minus26);
                LOGD(TAG, "chroma_qp_index_offset = %d", pps->chroma_qp_index_offset);
                LOGD(TAG, "deblocking_filter_control_present_flag = %d", pps->deblocking_filter_control_present_flag);
                delete pps;
            } else {
                LOGE(TAG, "failed to parse pps");
            }
        }
        delete nal;
        nal = nullptr;
    }
}
