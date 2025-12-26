//
// Created by zu on 2025/12/25.
//

#include "BufferByteReader.h"

BufferByteReader::BufferByteReader(const uint8_t *data, int64_t size) {
    this->buffer = data;
    this->size = size;
    currentPos = 0;
}

uint8_t BufferByteReader::nextByte() {
    if (currentPos < size) {
        uint8_t d = buffer[currentPos];
        return d;
    } else {
        return 0;
    }
}

uint32_t BufferByteReader::nextBytes(int n) {
    int count = n;
    if (count > 4) {
        count = 4;
    }
    if (count < 0) {
        count = 0;
    }
    int64_t remainCount = remainByteCount();
    if (count > remainCount) {
        count = remainCount;
    }
    uint32_t ret = 0;
    for (int i = 0; i < count; i++) {
        uint8_t b = buffer[currentPos + i];
        ret = (ret << 8) | (b & 0x00FF);
    }
    return ret;
}

uint8_t BufferByteReader::readByte() {
    if (currentPos < size) {
        uint8_t d = buffer[currentPos];
        currentPos++;
        return d;
    } else {
        return 0;
    }
}

uint32_t BufferByteReader::readBytes(int n) {
    int count = n;
    if (count > 4) {
        count = 4;
    }
    if (count < 0) {
        count = 0;
    }
    int64_t remainCount = remainByteCount();
    if (count > remainCount) {
        count = remainCount;
    }
    uint32_t ret = 0;
    for (int i = 0; i < count; i++) {
        uint8_t b = readByte();
        ret = (ret << 8) | (b & 0x00FF);
    }
    return ret;
}

int64_t BufferByteReader::remainByteCount() {
    return size - currentPos;
}

bool BufferByteReader::hasMoreBytes() {
    return currentPos < size;
}

void BufferByteReader::reset() {
    currentPos = 0;
}
