//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/24.
//


#include "stream.h"

void Stream::PutPacket(AVPacket *pkt) {
    int ret;
    std::unique_lock<std::mutex> lock(mtx_);
    while (true) {
        if (queue_.size() < MAX_SIZE) {
            AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
            if (packet == NULL) {
                av_log(NULL, AV_LOG_FATAL, "Could not create new AVPacket.\n");
            } else {
                ret = av_copy_packet(packet, pkt);
                if (ret != 0) {
                    av_log(NULL, AV_LOG_FATAL, "Could not copy AVPacket.\n");
                } else {
                    queue_.push(*packet);
                    break;
                }
            }
        }else {
            full_.wait(lock);
        }
    }
}

Stream::Stream(int index, const AVFormatContext *ctx) : stream_index_(index) {
    if (ctx == nullptr || index < 0 || index >= ctx->nb_streams) return;
    AVCodec *codec;
    int ret = 0;

    avctx = avcodec_alloc_context3(NULL);
    if (!avctx) return;
    ret = avcodec_parameters_to_context(avctx, ctx->streams[index]->codecpar);
    if (ret < 0) {
        avcodec_free_context(&avctx);
        return;
    }
    av_codec_set_pkt_timebase(avctx, ctx->streams[index]->time_base);
    codec = avcodec_find_decoder(avctx->codec_id);
    avctx->codec_id = codec->id;
    ret = avcodec_open2(avctx, codec, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_FATAL, "Fail to open codec on stream %d\n", index);
        avcodec_free_context(&avctx);
    }
}
