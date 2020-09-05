#include "methods.h"
#include "ffmpeg_utils.h"
#include <iostream>
#include <fcgi_stdio.h>

extern "C" {
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libavutil/log.h>
#include<libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

void save2File(AVFrame *p_frame, HandleResult *result) {
    AVFormatContext *p_format_ctx = avformat_alloc_context();
    std::string mine_type("image/");
    mine_type.append(result->src_codec_name);
    try{
        p_format_ctx->oformat = av_guess_format(result->src_codec_name, nullptr, nullptr);
    } catch (...) {
        return_error_info("av_guess_format 异常了.");
        return;
    }
    if (p_format_ctx->oformat == nullptr) {
        std::string errorMsg("没找到类型:");
        errorMsg.append(result->src_format_ctx->iformat->mime_type);
    } else {
        char out_path[200];
        time_t now;
        time(&now);
        sprintf(out_path, "/Users/gloomypan/www/%ld.%s", now, result->src_codec->name);

        int ret = avio_open(&p_format_ctx->pb, out_path, AVIO_FLAG_READ_WRITE);     // NOLINT(hicpp-signed-bitwise)
        if (ret == 0) {
            //success
            AVStream *p_av_stream = avformat_new_stream(p_format_ctx, nullptr);
            AVCodecContext *p_codec_ctx = p_av_stream->codec;

            p_codec_ctx->codec_id = result->src_codec->id;
            p_codec_ctx->codec_type = result->src_codec->type;
            p_codec_ctx->pix_fmt = result->src_codec_ctx->pix_fmt;
            p_codec_ctx->time_base.num = result->src_codec_ctx->time_base.num;
            p_codec_ctx->time_base.den = result->src_codec_ctx->time_base.den;

            p_codec_ctx->width = result->dest_width;
            p_codec_ctx->height = result->dest_height;


            av_dump_format(p_format_ctx, 0, out_path, 1);


            AVCodec *p_codec = avcodec_find_encoder(p_codec_ctx->codec_id);

            if (!avcodec_is_open(p_codec_ctx)) {
                ret = avcodec_open2(p_codec_ctx, p_codec, nullptr);
            }

            if (ret == 0) {
                avformat_write_header(p_format_ctx, nullptr);

                int y_size = p_codec_ctx->width * p_codec_ctx->height;
                AVPacket pkt;
                av_new_packet(&pkt, y_size * 3);

                avcodec_send_frame(p_codec_ctx, p_frame);

                int got_picture = avcodec_receive_packet(p_codec_ctx, &pkt);

                if (got_picture == 0) {
                    av_write_frame(p_format_ctx, &pkt);
                    av_packet_unref(&pkt);
                    av_write_trailer(p_format_ctx);
                }
            }
        }
    }

    avformat_flush(p_format_ctx);
    avformat_close_input(&p_format_ctx);
}

void operate(AVFrame *p_frame, HandleResult *result) {
    int out_width = 384;
    int out_height = 216;
    result->dest_width = out_width;
    result->dest_height = out_height;
    //获得解码结果,然后缩放图片
    SwsContext *p_sws_ctx = sws_getContext(result->src_width, result->src_height, result->src_fmt,
                                           out_width, out_height, result->src_fmt,
                                           SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    int dst_frame_size = av_image_get_buffer_size(result->src_fmt, out_width, out_height, 1);
    auto *outBuff = (uint8_t *) av_malloc(dst_frame_size);
    AVFrame *frame = av_frame_alloc();
    av_image_fill_arrays(frame->data, frame->linesize, outBuff, result->src_fmt, out_width, out_height, 1);

    int ret = sws_scale(p_sws_ctx,
                        p_frame->data, p_frame->linesize,
                        0, result->src_height,
                        frame->data, frame->linesize);
    if (ret > 0) {
        save2File(frame, result);
    }

    av_frame_unref(frame);
}

void return_image_scale(const char *uri) {
    std::string path(FILE_PREFIX);
    path.append(uri);
    auto *result = new HandleResult();
    //解码
    ff_decode(path.c_str(), result, operate);

}