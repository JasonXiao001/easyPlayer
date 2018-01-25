//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/24.
//


#include "stream.h"

void Stream::PutPacket(AVPacket &pkt) {
    std::unique_lock<std::mutex> lock(mtx_);
    packet_full_.wait(lock, [this] { return packet_queue_.size() < PKT_MAX_SIZE; });
    packet_queue_.push(pkt);
    packet_empty_.notify_one();
}

void Stream::GetPacket(AVPacket &pkt) {
    std::unique_lock<std::mutex> lock(mtx_);
    packet_empty_.wait(lock, [this] { return !packet_queue_.empty(); });
    pkt = packet_queue_.front();
    packet_queue_.pop();
    packet_full_.notify_one();
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

void Stream::decode() {
    AVPacket pkt;
    int ret;
    do {
        GetPacket(pkt);
        if (pkt.data == NULL) {
            av_log(NULL, AV_LOG_FATAL, "reach eof.\n");
            break;
        }
        ret = avcodec_send_packet(avctx, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
            break;
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avctx, frame);
        if (ret < 0 && ret != AVERROR_EOF)
            break;
        frame->pts = av_frame_get_best_effort_timestamp(frame);
        frame_queue.put_frame(frame);
        if (ret < 0) {
            packet_pending = 0;
        } else {

        }
    } while (true);

}


