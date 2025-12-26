//
// Created by zu on 2025/12/25.
//

#ifndef LB_CAMERA_SDK_V2_BUFFERBYTEREADER_H
#define LB_CAMERA_SDK_V2_BUFFERBYTEREADER_H

#include "ByteReader.h"
#include <vector>

class BufferByteReader: public ByteReader {
public:
    BufferByteReader(const uint8_t *data, int64_t size);

    uint8_t nextByte() override;

    uint32_t nextBytes(int n) override;

    uint8_t readByte() override;

    uint32_t readBytes(int n) override;

    int64_t remainByteCount() override;

    bool hasMoreBytes() override;

    void reset() override;

private:
    const uint8_t *buffer;
    int64_t size = 0;
    int64_t currentPos = 0;
};


#endif //LB_CAMERA_SDK_V2_BUFFERBYTEREADER_H
