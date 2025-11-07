cmake_minimum_required(VERSION 4.0)
project(Analyze_H264_SPS)

set(CMAKE_CXX_STANDARD 11)

add_executable(Analyze_H264_SPS main.cpp
        FileBitReader.cpp
        FileBitReader.h
        Log.h
        h264_nal.cpp
        h264_nal.h
        BufferBitReader.cpp
        BufferBitReader.h
        SPS.h
        NAL.h
        VUI.h
        HRD.h)
