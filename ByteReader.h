//
// Created by zu on 2025/12/25.
//

#ifndef LB_CAMERA_SDK_V2_BYTEREADER_H
#define LB_CAMERA_SDK_V2_BYTEREADER_H

#include <stdlib.h>

class ByteReader {
public:
    virtual uint8_t nextByte() = 0;
    virtual uint32_t nextBytes(int n) = 0;
    virtual uint8_t readByte() = 0;
    virtual uint32_t readBytes(int n) = 0;
    virtual int64_t remainByteCount() = 0;
    virtual bool hasMoreBytes() = 0;
    virtual void reset() = 0;
};

#endif //LB_CAMERA_SDK_V2_BYTEREADER_H
