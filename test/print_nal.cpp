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
        LOGD(TAG, "nal_unit_type = %d, nal_ref_idc = %d", nal->nal_unit_type, nal->nal_ref_idc);
        if (nal->nal_unit_type == 7) {
            LOGD(TAG, "find sps unit, sps size = %d", nal->rbsp.size());
            H264_SPS *sps = h264_parse_sps(nal);
            if (sps != nullptr) {
                LOGD(TAG, "seq_parameter_set_id = %d", sps->seq_parameter_set_id);
                if (sps->vui != nullptr) {
                    H264_VUI *vui = sps->vui;
                    if (vui->timing_info_present_flag) {
                        LOGD(TAG, "vui.time_scale = %d", vui->time_scale);
                        LOGD(TAG, "vui.num_units_in_tick = %d", vui->num_units_in_tick);
                        LOGD(TAG, "fps = %d", vui->time_scale / (2 * vui->num_units_in_tick));
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
