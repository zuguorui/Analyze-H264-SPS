//
// Created by WangXi on 2025/11/6.
//

#ifndef ANALYZE_H264_SPS_LOG_H
#define ANALYZE_H264_SPS_LOG_H

#include <stdio.h>

#define LOGD(TAG, FORMAT, ...) \
do { \
fprintf(stdout, "[%s] " FORMAT "\n", TAG, ##__VA_ARGS__); \
fflush(stdout); \
} while(0)

#define LOGE(TAG, FORMAT, ...) \
do { \
fprintf(stderr, "[%s] " FORMAT "\n", TAG, ##__VA_ARGS__); \
fflush(stderr); \
} while(0)

#endif //ANALYZE_H264_SPS_LOG_H