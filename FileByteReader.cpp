//
// Created by zu on 2025/12/26.
//

#include "FileByteReader.h"

#include <memory.h>
#include <algorithm>

#include "Log.h"

#define TAG "FileByteReader"

using namespace std;

FileByteReader::FileByteReader(const char *path) {
    file = fopen(path, "rb");
    if (file == nullptr) {
        LOGE(TAG, "Could not open file %s", path);
        return;
    }
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    bufferSize = fread(buffer, sizeof(uint8_t), BUFFER_CAPACITY, file);
    bufferPosInFile = 0L;
    bytePosInBuffer = 0L;
}

FileByteReader::~FileByteReader() {
    if (file != nullptr) {
        fclose(file);
    }
}

int64_t FileByteReader::bufferForward() {
    if (file == nullptr) {
        return 0;
    }
    if (bytePosInBuffer < BUFFER_CAPACITY / 2) {
        return 0;
    }
    memmove(buffer, buffer + BUFFER_CAPACITY / 2, BUFFER_CAPACITY / 2);
    bytePosInBuffer -= BUFFER_CAPACITY / 2;
    bufferSize -= BUFFER_CAPACITY / 2;
    bufferPosInFile += BUFFER_CAPACITY / 2;

    int64_t toReadSize = min(BUFFER_CAPACITY - bufferSize, fileSize - (bufferPosInFile + bufferSize));
    if (toReadSize <= 0) {
        return 0;
    }
    int64_t actualReadSize = fread(buffer + bufferSize, sizeof(uint8_t), toReadSize, file);
    bufferSize += actualReadSize;
    return actualReadSize;
}

uint32_t FileByteReader::nextBytes(int n) {
    if (n <= 0) {
        return 0;
    }
    if (n > 4) {
        n = 4;
    }

    if (bytePosInBuffer + n > bufferSize) {
        bufferForward();
    }

    int64_t remain = remainByteCount();
    if (n > remain) {
        n = remain;
    }

    uint32_t ret = 0;
    for (int i = 0; i < n; i++) {
        ret = (ret << 8) | (buffer[bytePosInBuffer + i] & 0x00FF);
    }
    return ret;
}

uint8_t FileByteReader::nextByte() {
    return nextBytes(1);
}

uint32_t FileByteReader::readBytes(int n) {
    if (n <= 0) {
        return 0;
    }
    if (n > 4) {
        n = 4;
    }

    if (bytePosInBuffer + n > bufferSize) {
        bufferForward();
    }

    int64_t remain = remainByteCount();
    if (n > remain) {
        n = remain;
    }

    uint32_t ret = 0;
    for (int i = 0; i < n; i++) {
        ret = (ret << 8) | (buffer[bytePosInBuffer++] & 0x00FF);
    }
    return ret;
}

uint8_t FileByteReader::readByte() {
    return readBytes(1);
}

bool FileByteReader::hasMoreBytes() {
    return remainByteCount() > 0;
}

int64_t FileByteReader::remainByteCount() {
    return fileSize - bufferPosInFile - bytePosInBuffer;
}

void FileByteReader::reset() {
    if (file == nullptr) {
        return;
    }
    fseek(file, 0, SEEK_SET);
    bufferPosInFile = 0L;
    bytePosInBuffer = 0L;

    bufferSize = fread(buffer, sizeof(uint8_t), BUFFER_CAPACITY, file);
}


