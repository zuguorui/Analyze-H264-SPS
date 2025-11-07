//
// Created by WangXi on 2025/11/6.
//

#ifndef ANALYZE_H264_SPS_BUFFERBITREADER_H
#define ANALYZE_H264_SPS_BUFFERBITREADER_H

#include <cstdint>
#include <stdlib.h>
#include <vector>

class BufferBitReader {
public:
    BufferBitReader(std::vector<uint8_t> &vec);

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

    size_t getSize();

private:
    int64_t bitPosInBuffer = 0L;
    std::vector<uint8_t> buffer;
};


#endif //ANALYZE_H264_SPS_BUFFERBITREADER_H