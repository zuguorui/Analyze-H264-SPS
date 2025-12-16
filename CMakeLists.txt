cmake_minimum_required(VERSION 4.0)
project(Analyze_H264_SPS)

set(CMAKE_CXX_STANDARD 17)

include_directories(./ /opt/homebrew/include/)

link_directories(/opt/homebrew/lib/)

file(GLOB SRC_FILES *.cpp)

add_executable(Analyze_H264_SPS
        ${SRC_FILES})

target_link_libraries(
        Analyze_H264_SPS
        avutil
        avdevice
        avformat
        avcodec
        avfilter
)
