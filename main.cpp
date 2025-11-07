#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cmath>
#include "h264_nal.h"
#include "log.h"

#define TAG "MAIN"
// ---------------------- main (for testing) ----------------------
using namespace std;
int main() {
    const char* path = "C:\\Users\\WangXi\\Downloads\\top_raw_2025-11-04_16-41-26.h264";
    FileBitReader reader(path);
    NAL *nal = parse_nal(reader);
    SPS *sps = parse_sps(nal);
    if (sps != nullptr && sps->vui != nullptr) {
        VUI *vui = sps->vui;
        if (vui->timing_info_present_flag) {
            LOGD(TAG, "vui.time_scale = %d", vui->time_scale);
            LOGD(TAG, "vui.num_units_in_tick = %d", vui->num_units_in_tick);
            LOGD(TAG, "fps = %d", vui->time_scale / (2 * vui->num_units_in_tick));
        }
    }
    delete nal;
    delete sps;
    return 0;
}
