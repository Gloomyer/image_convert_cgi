//
// Created by Gloomy Pan on 2020/9/5.
//

extern "C" {
#include <libavformat/avformat.h>
};

#ifndef CONVERT_CGI_FFMPEG_UTILS_H
#define CONVERT_CGI_FFMPEG_UTILS_H

typedef struct HandleResult {
    int code = 0;
    int src_width = 0;
    int src_height = 0;
    int dest_width = 0;
    int dest_height = 0;
    AVPixelFormat src_fmt;
    AVFormatContext *src_format_ctx = {};
    AVCodecContext *src_codec_ctx = {};
    AVCodec *src_codec = {};
    void *params1{};
    void *params2{};
    void *params3{};
    void *params4{};
    const char *data = {};
    const char *src_codec_name = {};
    void *ret = {};
} HandleResult;

/**
 * ffmpeg 解码
 * @param path path
 * @param callback callback
 */
void ff_decode(const char *path, HandleResult *result, void(*callback)(AVFrame *, HandleResult *result)) {
    AVFormatContext *p_format_ctx = nullptr;
    int ret = avformat_open_input(&p_format_ctx, path, nullptr, nullptr);
    if (ret == 0) {
        ret = avformat_find_stream_info(p_format_ctx, nullptr);
        if (ret == 0) {
            result->src_format_ctx = p_format_ctx;
            int video_stream_idx = -1;
            for (int i = 0; i < p_format_ctx->nb_streams; i++) {
                if (p_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    video_stream_idx = i;
                    break;
                }
            }

            if (video_stream_idx >= 0) {
                AVCodecContext *p_codec_ctx = p_format_ctx->streams[video_stream_idx]->codec;
                AVCodec *p_codec = avcodec_find_decoder(p_codec_ctx->codec_id);
                ret = avcodec_open2(p_codec_ctx, p_codec, nullptr);

                if (ret == 0) {
                    result->src_codec_name = p_codec->name;
                    result->src_width = p_codec_ctx->width;
                    result->src_height = p_codec_ctx->height;
                    result->src_fmt = p_codec_ctx->pix_fmt;
                    result->src_codec_ctx = p_codec_ctx;
                    result->src_codec = p_codec;

                    AVFrame *origin_frame = av_frame_alloc();
                    AVPacket packet;
                    av_init_packet(&packet);
                    if (origin_frame) {
                        while (av_read_frame(p_format_ctx, &packet) >= 0) {
                            if (packet.stream_index == video_stream_idx) {
                                avcodec_send_packet(p_codec_ctx, &packet);
                                int isReceived = avcodec_receive_frame(p_codec_ctx, origin_frame);
                                if (isReceived == 0) {
                                    callback(origin_frame, result);
                                }
                            }
                        }
                        av_packet_unref(&packet);
                        av_frame_unref(origin_frame);
                    }
                    avcodec_close(p_codec_ctx);
                }
            }
        }
        avformat_close_input(&p_format_ctx);
    }
}

#endif //CONVERT_CGI_FFMPEG_UTILS_H
