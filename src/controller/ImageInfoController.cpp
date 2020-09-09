#include "../Apis.h"
#include "../common/utils.h"
#include <iostream>
#include <sys/stat.h>
#include <fcgi_stdio.h>
#include <openssl/md5.h>

extern "C" {
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libavutil/log.h>
#include<libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

using namespace std;

typedef struct ImageInfo {
    long file_size{};
    int width{};
    int height{};
    int light{};
    const char *mine_type{};
    std::string *md5 = nullptr;
} ImageInfo;


void get_file_md5(struct stat &info, ImageInfo *image_info, std::string &in_file_path) {
    FILE *file = FCGI_fopen(in_file_path.c_str(), "rb+");
    if (file) {
        MD5_CTX md5Context;
        MD5_Init(&md5Context);
        char buf[info.st_size];
        FCGI_fread(buf, info.st_size, 1, file);
        MD5_Update(&md5Context, buf, info.st_size);
        unsigned char result[MD5_DIGEST_LENGTH];
        MD5_Final(result, &md5Context);
        char hex[35];
        memset(hex, 0, sizeof(hex));
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
            sprintf(hex + i * 2, "%02x", result[i]);
        }
        hex[32] = '\0';
        image_info->md5 = new std::string(hex);
        FCGI_fclose(file);
    }
}

void getImageInfo(FCGX_Request &request, ImageInfo *image_info, std::string &uri, bool isDetail) {
    auto start_time = std::chrono::system_clock::now();
    struct stat info{};
    std::string file_path(FILE_PREFIX);
    file_path.append(uri);
    stat(file_path.c_str(), &info);

    image_info->file_size = info.st_size;
    image_info->light = 0;
    image_info->mine_type = "";

    AVFormatContext *p_format_ctx = nullptr;
    int ret = avformat_open_input(&p_format_ctx, file_path.c_str(), nullptr, nullptr);
    if (ret == 0) {
        ret = avformat_find_stream_info(p_format_ctx, nullptr);
        if (ret == 0) {
            int video_stream_idx = -1;
            for (int i = 0; i < p_format_ctx->nb_streams; i++) {
                if (p_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    video_stream_idx = i;
                    break;
                }
            }
            if (video_stream_idx >= 0) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                AVCodecContext *p_codec_ctx = p_format_ctx->streams[video_stream_idx]->codec;
#pragma clang diagnostic pop
                AVCodec *p_codec = avcodec_find_decoder(p_codec_ctx->codec_id);
                image_info->mine_type = p_codec->name;
                ret = avcodec_open2(p_codec_ctx, p_codec, nullptr);
                if (ret == 0) {
                    image_info->width = p_codec_ctx->width;
                    image_info->height = p_codec_ctx->height;
                    if (isDetail) {
                        AVFrame *origin_frame = av_frame_alloc();
                        if (origin_frame) {
                            AVPacket packet;
                            av_init_packet(&packet);

                            while (av_read_frame(p_format_ctx, &packet) >= 0) {
                                if (packet.stream_index == video_stream_idx) {
                                    avcodec_send_packet(p_codec_ctx, &packet);
                                    int isReceived = avcodec_receive_frame(p_codec_ctx, origin_frame);
                                    if (isReceived == 0) {
                                        AVFrame *yuv_frame;
                                        if (p_codec_ctx->pix_fmt == AV_PIX_FMT_YUVJ444P) {
                                            yuv_frame = origin_frame;
                                        } else {
                                            SwsContext *pSwsCtx = sws_getContext(image_info->width, image_info->height,
                                                                                 p_codec_ctx->pix_fmt,
                                                                                 image_info->width, image_info->height,
                                                                                 AV_PIX_FMT_YUVJ444P,
                                                                                 SWS_FAST_BILINEAR,
                                                                                 nullptr, nullptr, nullptr);
                                            int dst_frame_size = av_image_get_buffer_size(AV_PIX_FMT_YUVJ444P,
                                                                                          image_info->width,
                                                                                          image_info->height,
                                                                                          1);
                                            auto *outBuff = (uint8_t *) av_malloc(dst_frame_size);
                                            yuv_frame = av_frame_alloc();
                                            av_image_fill_arrays(yuv_frame->data, yuv_frame->linesize, outBuff,
                                                                 AV_PIX_FMT_YUVJ444P,
                                                                 image_info->width, image_info->height, 1);
                                            sws_scale(pSwsCtx, origin_frame->data, origin_frame->linesize,
                                                      0, image_info->height,
                                                      yuv_frame->data, yuv_frame->linesize);
                                            av_frame_unref(origin_frame);
                                            origin_frame = yuv_frame;
                                        }

                                        uint8_t *buffer = yuv_frame->data[0];
                                        long total = 0;
                                        for (int i = 0; i < yuv_frame->linesize[0]; ++i) {
                                            total += buffer[i];
                                        }
                                        image_info->light = (int) (total / yuv_frame->linesize[0]);
                                    }
                                }
                            }
                            av_frame_unref(origin_frame);
                            av_packet_unref(&packet);
                        }
                    }
                    avcodec_close(p_codec_ctx);
                }
            }
        }
        avformat_close_input(&p_format_ctx);
    }

    if (isDetail) {
        get_file_md5(info, image_info, file_path);
    }

    auto end_time = std::chrono::system_clock::now();
    auto duration = (std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time)).count();
    FCGX_FPrintF(request.out, "Time-Consuming: %lld;\r\n", duration);
}

void controller_ret_image_basic_info(FCGX_Request &request, std::string &uri) {
    ImageInfo info;
    getImageInfo(request, &info, uri, false);
    FCGX_FPrintF(request.out, "Content-type: application/json;\r\n\r\n");
    FCGX_FPrintF(request.out, R"(
{
    "fileSize": %ld,
    "width": %d,
    "height": %d,
    "mineType": "%s"
})",
                 info.file_size, info.width, info.height, info.mine_type);
}

void controller_ret_image_detail_info(FCGX_Request &request, std::string &uri) {
    ImageInfo info;
    getImageInfo(request, &info, uri, true);
    FCGX_FPrintF(request.out, "Content-type: application/json;\r\n\r\n");
    FCGX_FPrintF(request.out, R"(
{
    "fileSize": %ld,
    "width": %d,
    "height": %d,
    "light": %d,
    "mineType": "%s",
    "md5": "%s"
})",
                 info.file_size, info.width, info.height, info.light, info.mine_type, info.md5->c_str());
    delete info.md5;

}