#include <iostream>

#include "mux_flv.h"
#include "print_nal.h"

#define TAG "MAIN"

using namespace std;
int main() {
    const char* path = "/Users/zu/Downloads/TOP_1920x1080_30fps.h264";
    //print_nal(path);
    mux_flv_avcc(path, "/Users/zu/work_space/CPPProject/Analyze-H264-SPS/out.flv");
    return 0;
}
