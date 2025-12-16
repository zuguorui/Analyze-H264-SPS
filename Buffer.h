//
// Created by zu on 2025/12/16.
//

#ifndef ANALYZE_H264_SPS_BUFFER_H
#define ANALYZE_H264_SPS_BUFFER_H

#include <cstdlib>
#include <cstring>

struct Buffer {
    uint8_t *data = nullptr;
    int64_t capacity = 0;
    int64_t size = 0;

    explicit Buffer(int capacity) {
        if (capacity <= 0) {
            throw "capacity must > 0";
        }
        this->capacity = capacity;
        data = new uint8_t[capacity];
        size = 0;
    }

    Buffer(const Buffer &src) {
        this->capacity = src.capacity;
        this->size = src.size;
        this->data = new uint8_t[this->capacity];
        memcpy(this->data, src.data, this->size);
    }

    Buffer(Buffer &&src) {
        this->capacity = src.capacity;
        this->size = src.size;
        this->data = src.data;
        src.data = nullptr;
    }

    ~Buffer() {
        if (data) {
            delete[] data;
            data = nullptr;
        }
    }
};

#endif //ANALYZE_H264_SPS_BUFFER_H