//
// Created by WangXi on 2025/11/6.
//

#ifndef ANALYZE_H264_SPS_H264_NAL_H
#define ANALYZE_H264_SPS_H264_NAL_H

#include "../BitReader.h"
#include "../ByteReader.h"
#include "h264_NAL.h"
#include "H264_SPS.h"
#include "H264_PPS.h"
#include <vector>

/**
 * 从流中解析出一个NAL单元。推荐使用ByteReader版本，更快。
 * @param reader 文件比特流读取器。解析是从reader的当前位置开始。
 * @return NAL，调用者需自己释放内存
 */
H264_NAL* h264_parse_nal(BitReader &reader);
/**
 * 从流中解析出一个NAL单元。
 * @param reader 文件比特流读取器。解析是从reader的当前位置开始。
 * @return NAL，调用者需自己释放内存
 */
H264_NAL* h264_parse_nal(ByteReader &reader);

/**
 * 从已经提取的NAL字节中解析NAL。要求不包含起始字节。
 * @param nalBody
 * @return
 */
H264_NAL* h264_parse_nal(std::vector<uint8_t> &nalBody);

/**
 * 从NAL中解析SPS
 * @param nal
 * @return
 */
H264_SPS* h264_parse_sps(H264_NAL *nal);

/**
 * 从NAL中解析PPS
 * @param nal
 * @return
 */
H264_PPS* h264_parse_pps(H264_NAL *nal);

/**
 * 从流中解析NAL字节数据。
 * @param reader 流读取器
 * @return NAL字节数据，去除起始字节和后缀0。不去除防竞争字节。
 */
std::vector<uint8_t> h264_parse_nal_body(BitReader &reader);

/**
 * 从流中解析NAL字节数据。
 * @param reader 流读取器
 * @return NAL字节数据，去除起始字节和后缀0。不去除防竞争字节。
 */
std::vector<uint8_t> h264_parse_nal_body(ByteReader &reader);

/**
 * 判断data是否是关键帧数据。判断依据是关键帧是第一个NAL是否是SPS/PPS/IDR。
 * @param data
 * @param size
 * @return
 */
bool h264_is_key_frame(uint8_t *data, int size);

/**
 * 判断NAL类型。
 * @param nalBody NAL字节数据。要求不包含起始字节。
 * @return
 */
int h264_get_nal_type(std::vector<uint8_t> &nalBody);

#endif //ANALYZE_H264_SPS_H264_NAL_H