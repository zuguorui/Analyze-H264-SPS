//
// Created by WangXi on 2025/11/6.
//

#ifndef ANALYZE_H264_SPS_FILEBITSTREAMREADER_H
#define ANALYZE_H264_SPS_FILEBITSTREAMREADER_H

#include <cstdint>
#include <stdlib.h>
#include <wchar.h>

class FileBitReader {
public:
    explicit FileBitReader(const char* path);
    ~FileBitReader();

    uint32_t readBits(int bits);
    uint32_t nextBits(int bits);

    uint8_t readByte();
    uint8_t nextByte();

    bool byteAligned();
    bool moreDataInByteStream();

    uint32_t f(int bits);
    uint32_t u(int bits);
    uint32_t b(int bits);
    int32_t i(int bits);
    uint32_t ue();
    int32_t se();

    void reset();


private:
    static constexpr int BUFFER_CAPACITY = 4096;
    uint8_t buffer[BUFFER_CAPACITY];

    FILE *file = nullptr;
    // 缓冲区头部在字节中的位置，单位byte
    int64_t bufferPosInFile = 0L;
    // 读取指针在缓冲区的位置，单位bit。
    int64_t bitPosInBuffer = 0L;

    // 缓冲区的有效大小，单位byte。针对文件过小的情况
    int64_t bufferSize = 0L;
    // 文件大小，单位byte
    int64_t fileSize = 0L;

    // 缓冲区向后移动，返回值是移动的位置，单位byte
    int64_t bufferForward();
};


#endif //ANALYZE_H264_SPS_FILEBITSTREAMREADER_H