//
// Created by zu on 2025/12/26.
//

#ifndef ANALYZE_H264_SPS_FILEBYTEREADER_H
#define ANALYZE_H264_SPS_FILEBYTEREADER_H

#include "ByteReader.h"
#include <stdio.h>

class FileByteReader: public ByteReader {
public:
    FileByteReader(const char* path);
    ~FileByteReader();

    uint8_t nextByte() override;

    uint32_t nextBytes(int n) override;

    uint8_t readByte() override;

    uint32_t readBytes(int n) override;

    int64_t remainByteCount() override;

    bool hasMoreBytes() override;

    void reset() override;
private:
    static constexpr int BUFFER_CAPACITY = 4096;
    uint8_t buffer[BUFFER_CAPACITY];

    FILE *file = nullptr;

    int64_t bufferPosInFile = 0;
    int64_t bytePosInBuffer = 0;

    int64_t bufferSize = 0;
    int64_t fileSize = 0;

    int64_t bufferForward();
};


#endif //ANALYZE_H264_SPS_FILEBYTEREADER_H