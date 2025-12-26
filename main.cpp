#include <iostream>

#include "test/mux.h"
#include "test/print_nal.h"

#define TAG "MAIN"

using namespace std;
int main() {
    const char* path = "/Users/zu/Downloads/TOP_1920x1080_30fps.h264";
    //print_nal(path);
    //mux_h264_flv(path, "/Users/zu/Downloads/out.flv");
    mux_h264(path, "/Users/zu/Downloads/out.mov", "mov");
    return 0;
}
