#include <iostream>

#include "FileByteReader.h"
#include "Log.h"
#include "H264/h264_analyze.h"
#include "test/mux.h"
#include "test/print_nal.h"

#define TAG "MAIN"

using namespace std;
int main() {
    const char* path = "/Users/zu/Downloads/TOP_1920x1080_30fps.h264";
    print_nal(path);
    //mux_h264_flv(path, "/Users/zu/Downloads/out.flv");
    //mux_h264(path, "/Users/zu/Downloads/out.mov", "mov");
    // vector<uint8_t> nalData;
    // vector<uint8_t> pureNalData;
    // FileByteReader reader(path);
    // int ret = 0;
    // int type = 0;
    // int isKeyFrame = 0;
    // int index = 0;
    // while (reader.hasMoreBytes()) {
    //     ret = h264_parse_raw_nal(reader, nalData);
    //     if (ret != 0) {
    //         LOGE(TAG, "index: %d, ret = %d", index, ret);
    //         break;
    //     }
    //     pureNalData.clear();
    //     if (nalData[2] == 0x01) {
    //         pureNalData.insert(pureNalData.begin(), nalData.begin() + 3, nalData.end());
    //     } else {
    //         pureNalData.insert(pureNalData.begin(), nalData.begin() + 4, nalData.end());
    //     }
    //     type = h264_get_nal_type(pureNalData);
    //     isKeyFrame = h264_is_key_frame(nalData.data(), nalData.size());
    //     LOGD(TAG, "index: %d, type = %d, isKeyFrame = %d, nalSize = %ld", index, type, isKeyFrame, nalData.size());
    //     index++;
    // }
    // LOGD(TAG, "totalNal: %d", index);
    return 0;
}
