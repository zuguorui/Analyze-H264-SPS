cmake_minimum_required(VERSION 3.28.1)
project(Analyze_H264_SPS)

set(CMAKE_CXX_STANDARD 17)

include_directories(./ /opt/homebrew/include/)

link_directories(/opt/homebrew/lib/)

file(GLOB SRC_FILES *.cpp test/*.cpp H264/*.cpp)

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
