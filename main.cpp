#include <iostream>

#include "mux_flv.h"
#include "print_nal.h"

#define TAG "MAIN"

using namespace std;
int main() {
    const char* path = "/Users/zu/Downloads/2025-12-16_22-19-31_30FPS_1920x1080.h264";
    //print_nal(path);
    mux_flv_annexb(path, "/Users/zu/Downloads/out.flv");
    return 0;
}
