//
// Created by zu on 2025/12/16.
//

#include "mux_flv.h"
#include "h264_nal.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
}

#include "Log.h"

#define TAG "mux_flv"

using namespace std;

const AVRational MILLI_SECONDS_TIME_BASE{1, 1000};

uint8_t* build_h264_avcc_csd(uint8_t *sps, int spsSize, uint8_t *pps, int ppsSize, int *outSize) {
    if (!sps || !pps || spsSize < 4 || ppsSize < 1 || !outSize) {
        return nullptr;
    }

    // AVCDecoderConfigurationRecord size
    int size =
            5 +                 // fixed header
            1 + 2 + spsSize +    // SPS
            1 + 2 + ppsSize;     // PPS

    uint8_t* extradata = (uint8_t*)av_mallocz(
            size + AV_INPUT_BUFFER_PADDING_SIZE);
    if (!extradata) {
        return nullptr;
    }

    uint8_t* p = extradata;

    // configurationVersion
    *p++ = 0x01;

    // profile / compatibility / level
    *p++ = sps[1];   // AVCProfileIndication
    *p++ = sps[2];   // profile_compatibility
    *p++ = sps[3];   // AVCLevelIndication

    // lengthSizeMinusOne: 4 bytes NALU length
    *p++ = 0xFF;

    // numOfSequenceParameterSets (only 1 SPS)
    *p++ = 0xE1;

    // SPS length (big endian)
    *p++ = (spsSize >> 8) & 0xFF;
    *p++ = spsSize & 0xFF;

    // SPS data
    memcpy(p, sps, spsSize);
    p += spsSize;

    // numOfPictureParameterSets (1 PPS)
    *p++ = 0x01;

    // PPS length
    *p++ = (ppsSize >> 8) & 0xFF;
    *p++ = ppsSize & 0xFF;

    // PPS data
    memcpy(p, pps, ppsSize);

    *outSize = size;
    return extradata;
}

static uint8_t* annexb_nal_to_avcc(NAL *nal, int *outSize) {
    uint8_t *ret = new uint8_t[nal->rbsp.size() + 5];
    int nalSize = nal->rbsp.size() + 1;
    ret[0] = (nalSize >> 24) & 0x00FF;
    ret[1] = (nalSize >> 16) & 0x00FF;
    ret[2] = (nalSize >> 8) & 0x00FF;
    ret[3] = (nalSize) & 0x00FF;
    ret[4] = (uint8_t)((nal->forbidden_zero_bit << 7) | (nal->nal_ref_idc << 5) | (nal->nal_unit_type & 0x1F));
    memcpy(ret + 5, nal->rbsp.data(), nal->rbsp.size());
    *outSize = nal->rbsp.size() + 5;
    return ret;
}

void mux_flv_avcc(const char *h264Path, const char *flvPath) {

    // 查找sps和pps
    FileBitReader reader(h264Path);

    NAL *spsNAL = nullptr;
    NAL *ppsNAL = nullptr;
    double fps = 30;

    NAL *nal = nullptr;
    while ((nal = parse_nal(reader)) != nullptr) {
        if (nal->nal_unit_type == 7) {
            if (spsNAL) {
                delete spsNAL;
                spsNAL = nullptr;
            }
            spsNAL = nal;
            nal = nullptr;
            SPS *sps = parse_sps(spsNAL);
            if (sps->vui != nullptr) {
                VUI *vui = sps->vui;
                if (vui->timing_info_present_flag) {
                    LOGD(TAG, "vui.time_scale = %d", vui->time_scale);
                    LOGD(TAG, "vui.num_units_in_tick = %d", vui->num_units_in_tick);
                    if (vui->num_units_in_tick != 0) {
                        fps = vui->time_scale * 1.0 / (2 * vui->num_units_in_tick);
                        LOGD(TAG, "解析到fps = %f", fps);
                    }
                }
            }
        } else if (nal->nal_unit_type == 8) {
            if (ppsNAL) {
                delete ppsNAL;
                ppsNAL = nullptr;
            }
            ppsNAL = nal;
            nal = nullptr;
        } else {
            delete nal;
            nal = nullptr;
        }

        if (spsNAL && ppsNAL) {
            break;
        }
    }

    double interval = 1000.0 / fps;

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

    int spsDataSize = spsNAL->rbsp.size() + 1;
    uint8_t *spsData = new uint8_t[spsDataSize];
    spsData[0] = (uint8_t)((spsNAL->forbidden_zero_bit << 7) | (spsNAL->nal_ref_idc << 5) | spsNAL->nal_unit_type);
    memcpy(spsData + 1, spsNAL->rbsp.data(), spsNAL->rbsp.size());
    int ppsDataSize = ppsNAL->rbsp.size() + 1;
    uint8_t *ppsData = new uint8_t[ppsDataSize];
    ppsData[0] = (uint8_t)((ppsNAL->forbidden_zero_bit << 7) | (ppsNAL->nal_ref_idc << 5) | ppsNAL->nal_unit_type);
    memcpy(ppsData + 1, ppsNAL->rbsp.data(), ppsNAL->rbsp.size());
    int csdDataSize = 0;
    uint8_t *csdData = build_h264_avcc_csd(spsData, spsDataSize, ppsData, ppsDataSize, &csdDataSize);

    stream->codecpar->extradata = csdData;
    stream->codecpar->extradata_size = csdDataSize;

    if (avformat_write_header(formatCtx, nullptr) != 0) {
        LOGE(TAG, "write header failed");
        return;
    }

    reader.reset();
    int64_t lastPts = -1L;
    AVPacket *pkt = av_packet_alloc();
    while ((nal = parse_nal(reader)) != nullptr) {
        LOGD(TAG, "nal type %d", nal->nal_unit_type);
        if (nal->nal_unit_type == 7) {
            // if (spsNAL) {
            //     delete spsNAL;
            //     spsNAL = nullptr;
            // }
            // spsNAL = nal;
        } else if (nal->nal_unit_type == 8) {
            // if (ppsNAL) {
            //     delete ppsNAL;
            //     ppsNAL = nullptr;
            // }
            // ppsNAL = nal;
        } else if (nal->nal_unit_type == 5) {
            // IDR frame
            if (lastPts < 0) {
                lastPts = 0;
            } else {
                lastPts += interval;
            }

            //IDR前发送一次sps/pps
            int spsSize;
            uint8_t *spsData = annexb_nal_to_avcc(spsNAL, &spsSize);
            int ppsSize;
            uint8_t *ppsData = annexb_nal_to_avcc(ppsNAL, &ppsSize);
            av_new_packet(pkt, spsSize + ppsSize);
            memcpy(pkt->data, spsData, spsSize);
            memcpy(pkt->data + spsSize, ppsData, ppsSize);
            pkt->size = spsSize + ppsSize;

            pkt->stream_index = 0;
            pkt->pts = lastPts;
            pkt->dts = lastPts;
            pkt->time_base = MILLI_SECONDS_TIME_BASE;
            pkt->flags = 0;
            pkt->flags |= AV_PKT_FLAG_KEY;
            av_interleaved_write_frame(formatCtx, pkt);
            av_packet_unref(pkt);

            // 然后发送IDR数据
            int frameSize;
            uint8_t *frameData = annexb_nal_to_avcc(nal, &frameSize);
            av_new_packet(pkt, frameSize);
            memcpy(pkt->data, frameData, frameSize);
            pkt->size = frameSize;

            pkt->stream_index = 0;
            pkt->pts = lastPts;
            pkt->dts = lastPts;
            pkt->time_base = MILLI_SECONDS_TIME_BASE;
            pkt->flags = 0;
            pkt->flags |= AV_PKT_FLAG_KEY;

            av_interleaved_write_frame(formatCtx, pkt);
            av_packet_unref(pkt);
        } else {
            if (lastPts < 0) {
                lastPts = 0;
            } else {
                lastPts += interval;
            }
            int frameSize;
            uint8_t *frameData = annexb_nal_to_avcc(nal, &frameSize);

            av_new_packet(pkt, frameSize);
            memcpy(pkt->data, frameData, frameSize);
            pkt->size = frameSize;
            pkt->stream_index = 0;
            pkt->pts = lastPts;
            pkt->dts = lastPts;
            pkt->time_base = MILLI_SECONDS_TIME_BASE;
            pkt->flags = 0;
            // 如果是I帧，带上标志。这样ffmpeg的rtmp会在该帧附加SPS/PPS
            bool isKeyFrame = nal->nal_unit_type == 5;
            if (isKeyFrame) {
                pkt->flags |= AV_PKT_FLAG_KEY;
            }

            av_interleaved_write_frame(formatCtx, pkt);
            av_packet_unref(pkt);
        }
        delete nal;
        nal = nullptr;
    }

    av_interleaved_write_frame(formatCtx, nullptr);
    av_write_trailer(formatCtx);
    if (!(formatCtx->flags & AVFMT_NOFILE)) {
        avio_close(formatCtx->pb);
    }

    if (spsNAL) {
        delete spsNAL;
    }
    if (ppsNAL) {
        delete ppsNAL;
    }
    if (formatCtx) {
        avformat_free_context(formatCtx);
        formatCtx = nullptr;
    }
    av_packet_free(&pkt);
}

void mux_flv_annexb(const char *h264Path, const char *flvPath) {
    // 查找sps和pps
    FileBitReader reader(h264Path);

    vector<uint8_t> spsNAL;
    vector<uint8_t> ppsNAL;
    double fps = 30;

    while (true) {
        vector<uint8_t> nalBody = parse_nal_body(reader);
        if (nalBody.empty()) {
            break;
        }
        int nalType = get_nal_type(nalBody);
        if (nalType == 7) {
            LOGD(TAG, "解析到sps");
            spsNAL = nalBody;
            NAL *nal = parse_nal(nalBody);
            if (!nal) {
                continue;
            }
            SPS *sps = parse_sps(nal);
            if (!sps || !(sps->vui)) {
                continue;
            }
            VUI *vui = sps->vui;
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

    double interval = 1000.0 / fps;

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
        vector<uint8_t> nalBody = parse_nal_body(reader);
        if (nalBody.size() == 0) {
            break;
        }
        int nalType = get_nal_type(nalBody);
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