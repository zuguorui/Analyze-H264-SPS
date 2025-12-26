//
// Created by zu on 2025/12/16.
//

#include "test/mux.h"
#include "H264/h264_analyze.h"
#include "FileByteReader.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
}

#include "../Log.h"

#define TAG "mux_flv"

using namespace std;

const AVRational MILLI_SECONDS_TIME_BASE{1, 1000};

void mux_h264_flv(const char *h264Path, const char *flvPath) {
    // 查找sps和pps
    FileByteReader reader(h264Path);

    vector<uint8_t> spsNAL;
    vector<uint8_t> ppsNAL;
    double fps = 30;

    while (true) {
        vector<uint8_t> nalBody = h264_parse_nal_body(reader);
        if (nalBody.empty()) {
            break;
        }
        int nalType = h264_get_nal_type(nalBody);
        if (nalType == 7) {
            LOGD(TAG, "解析到sps");
            spsNAL = nalBody;
            H264_NAL *nal = h264_parse_nal(nalBody);
            if (!nal) {
                continue;
            }
            H264_SPS *sps = h264_parse_sps(nal);
            if (!sps || !(sps->vui)) {
                continue;
            }
            H264_VUI *vui = sps->vui;
            if (vui->timing_info_present_flag) {
                LOGD(TAG, "vui.time_scale = %d", vui->time_scale);
                LOGD(TAG, "vui.num_units_in_tick = %d", vui->num_units_in_tick);
                if (vui->num_units_in_tick != 0) {
                    fps = vui->time_scale * 1.0 / (2 * vui->num_units_in_tick);
                    LOGD(TAG, "解析到fps = %f", fps);
                }
            }
        } else if (nalType == 8) {
            LOGD(TAG, "解析到pps");
            ppsNAL = nalBody;
        }

        if (!spsNAL.empty() && !ppsNAL.empty()) {
            break;
        }
    }

    int64_t interval = (int64_t)(1.0 / fps / av_q2d(MILLI_SECONDS_TIME_BASE));

    AVFormatContext *formatCtx = nullptr;
    avformat_alloc_output_context2(&formatCtx, nullptr, "flv", flvPath);
    if (formatCtx == nullptr) {
        LOGE(TAG, "alloc formatCtx failed");
        return;
    }

    if (!(formatCtx->flags & AVFMT_NOFILE)) {
        avio_open(&formatCtx->pb, flvPath, AVIO_FLAG_WRITE);
    }

    AVStream *stream = avformat_new_stream(formatCtx, nullptr);
    if (!stream) {
        LOGE(TAG, "alloc stream failed");
        return;
    }

    stream->index = 0;
    stream->id = 0;
    stream->codecpar = avcodec_parameters_alloc();
    stream->codecpar->codec_id = AV_CODEC_ID_H264;
    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    stream->codecpar->width = 1920;
    stream->codecpar->height = 1080;
    stream->codecpar->format = AV_PIX_FMT_NV21;
    stream->codecpar->framerate = av_d2q(fps, 1000);

    int csdSize = spsNAL.size() + 4 + ppsNAL.size() + 4;
    uint8_t *csd = new uint8_t[csdSize];
    int dstIndex = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 1;
    memcpy(csd + dstIndex, spsNAL.data(), spsNAL.size());
    dstIndex += spsNAL.size();
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 1;
    memcpy(csd + dstIndex, ppsNAL.data(), ppsNAL.size());

    stream->codecpar->extradata = csd;
    stream->codecpar->extradata_size = csdSize;

    if (avformat_write_header(formatCtx, nullptr) != 0) {
        LOGE(TAG, "write header failed");
        return;
    }

    reader.reset();
    int64_t lastPts = -1L;
    AVPacket *pkt = av_packet_alloc();
    while (true) {
        vector<uint8_t> nalBody = h264_parse_nal_body(reader);
        if (nalBody.size() == 0) {
            break;
        }
        int nalType = h264_get_nal_type(nalBody);
        LOGD(TAG, "nalType=%d", nalType);
        if (nalType == 7) {
            spsNAL = nalBody;
        } else if (nalType == 8) {
            ppsNAL = nalBody;
        } else {
            if (lastPts < 0) {
                lastPts = 0;
            } else {
                lastPts += interval;
            }
            av_new_packet(pkt, nalBody.size() + 4);
            dstIndex = 0;
            pkt->data[dstIndex++] = 0;
            pkt->data[dstIndex++] = 0;
            pkt->data[dstIndex++] = 0;
            pkt->data[dstIndex++] = 1;
            memcpy(pkt->data + dstIndex, nalBody.data(), nalBody.size());
            pkt->size = nalBody.size() + 4;
            pkt->stream_index = 0;
            pkt->pts = lastPts;
            pkt->dts = lastPts;
            pkt->time_base = MILLI_SECONDS_TIME_BASE;
            pkt->flags = 0;
            if (nalType == 5) {
                pkt->flags |= AV_PKT_FLAG_KEY;
            }
            av_interleaved_write_frame(formatCtx, pkt);
            av_packet_unref(pkt);
        }
    }

    av_interleaved_write_frame(formatCtx, nullptr);
    av_write_trailer(formatCtx);
    if (!(formatCtx->flags & AVFMT_NOFILE)) {
        avio_close(formatCtx->pb);
    }
    if (formatCtx) {
        avformat_free_context(formatCtx);
        formatCtx = nullptr;
    }
    av_packet_free(&pkt);
}

void mux_h264(const char *h264Path, const char *outPath, const char *fmt) {
    // 查找sps和pps
    FileByteReader reader(h264Path);

    vector<uint8_t> spsNAL;
    vector<uint8_t> ppsNAL;
    double fps = 30;

    while (true) {
        vector<uint8_t> nalBody = h264_parse_nal_body(reader);
        if (nalBody.empty()) {
            break;
        }
        int nalType = h264_get_nal_type(nalBody);
        if (nalType == 7) {
            LOGD(TAG, "解析到sps");
            spsNAL = nalBody;
            H264_NAL *nal = h264_parse_nal(nalBody);
            if (!nal) {
                continue;
            }
            H264_SPS *sps = h264_parse_sps(nal);
            if (!sps || !(sps->vui)) {
                continue;
            }
            H264_VUI *vui = sps->vui;
            if (vui->timing_info_present_flag) {
                LOGD(TAG, "vui.time_scale = %d", vui->time_scale);
                LOGD(TAG, "vui.num_units_in_tick = %d", vui->num_units_in_tick);
                if (vui->num_units_in_tick != 0) {
                    fps = vui->time_scale * 1.0 / (2 * vui->num_units_in_tick);
                    LOGD(TAG, "解析到fps = %f", fps);
                }
            }
        } else if (nalType == 8) {
            LOGD(TAG, "解析到pps");
            ppsNAL = nalBody;
        }

        if (!spsNAL.empty() && !ppsNAL.empty()) {
            break;
        }
    }



    AVFormatContext *formatCtx = nullptr;
    avformat_alloc_output_context2(&formatCtx, nullptr, fmt, outPath);
    if (formatCtx == nullptr) {
        LOGE(TAG, "alloc formatCtx failed");
        return;
    }

    if (!(formatCtx->flags & AVFMT_NOFILE)) {
        avio_open(&formatCtx->pb, outPath, AVIO_FLAG_WRITE);
    }

    AVStream *stream = avformat_new_stream(formatCtx, nullptr);
    if (!stream) {
        LOGE(TAG, "alloc stream failed");
        return;
    }

    stream->index = 0;
    stream->id = 0;
    stream->time_base = MILLI_SECONDS_TIME_BASE;
    stream->codecpar = avcodec_parameters_alloc();
    stream->codecpar->codec_id = AV_CODEC_ID_H264;
    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    stream->codecpar->width = 1920;
    stream->codecpar->height = 1080;
    stream->codecpar->format = AV_PIX_FMT_NV21;
    stream->codecpar->framerate = av_d2q(fps, 1000);

    int csdSize = spsNAL.size() + 4 + ppsNAL.size() + 4;
    uint8_t *csd = new uint8_t[csdSize];
    int dstIndex = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 1;
    memcpy(csd + dstIndex, spsNAL.data(), spsNAL.size());
    dstIndex += spsNAL.size();
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 0;
    csd[dstIndex++] = 1;
    memcpy(csd + dstIndex, ppsNAL.data(), ppsNAL.size());

    stream->codecpar->extradata = csd;
    stream->codecpar->extradata_size = csdSize;

    LOGD(TAG, "stream->time_base: %d/%d", stream->time_base.num, stream->time_base.den);

    if (avformat_write_header(formatCtx, nullptr) != 0) {
        LOGE(TAG, "write header failed");
        return;
    }

    reader.reset();
    int64_t lastPts = -1L;
    int64_t interval = (int64_t)(1.0 / fps / av_q2d(stream->time_base));

    LOGD(TAG, "use framerate: %d/%d", stream->codecpar->framerate.num, stream->codecpar->framerate.den);
    LOGD(TAG, "stream->time_base: %d/%d", stream->time_base.num, stream->time_base.den);
    LOGD(TAG, "pts interval = %lld", interval);

    AVPacket *pkt = av_packet_alloc();
    while (true) {
        vector<uint8_t> nalBody = h264_parse_nal_body(reader);
        if (nalBody.size() == 0) {
            break;
        }
        int nalType = h264_get_nal_type(nalBody);
        //LOGD(TAG, "nalType=%d", nalType);
        if (nalType == 7) {
            spsNAL = nalBody;
        } else if (nalType == 8) {
            ppsNAL = nalBody;
        } else {
            if (lastPts < 0) {
                lastPts = 0;
            } else {
                lastPts += interval;
            }
            //LOGD(TAG, "pts = %lld", lastPts);
            av_new_packet(pkt, nalBody.size() + 4);
            dstIndex = 0;
            pkt->data[dstIndex++] = 0;
            pkt->data[dstIndex++] = 0;
            pkt->data[dstIndex++] = 0;
            pkt->data[dstIndex++] = 1;
            memcpy(pkt->data + dstIndex, nalBody.data(), nalBody.size());
            pkt->size = nalBody.size() + 4;
            pkt->stream_index = 0;
            pkt->pts = lastPts;
            pkt->dts = lastPts;
            pkt->time_base = MILLI_SECONDS_TIME_BASE;
            pkt->flags = 0;
            if (nalType == 5) {
                pkt->flags |= AV_PKT_FLAG_KEY;
            }
            av_interleaved_write_frame(formatCtx, pkt);
            av_packet_unref(pkt);
        }
    }

    av_interleaved_write_frame(formatCtx, nullptr);
    av_write_trailer(formatCtx);
    if (!(formatCtx->flags & AVFMT_NOFILE)) {
        avio_close(formatCtx->pb);
    }
    if (formatCtx) {
        avformat_free_context(formatCtx);
        formatCtx = nullptr;
    }
    av_packet_free(&pkt);
}