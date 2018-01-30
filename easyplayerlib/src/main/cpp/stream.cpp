//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/24.
//


#include <thread>
#include "stream.h"
#include "log_util.h"

void Stream::PutPacket(AVPacket &pkt) {
    std::unique_lock<std::mutex> lock(mtx_);
    packet_full_.wait(lock, [this] { return packet_queue_.size() < PKT_MAX_SIZE; });
    packet_queue_.push(pkt);
    packet_empty_.notify_one();
}

void Stream::GetPacket(AVPacket &pkt) {
    std::unique_lock<std::mutex> lock(mtx_);
    packet_empty_.wait(lock, [this] { return !packet_queue_.empty(); });
//    av_copy_packet(&pkt, &packet_queue_.front());
    pkt = packet_queue_.front();
    packet_queue_.pop();
    packet_full_.notify_one();
}

Stream::Stream(int index, const AVFormatContext *ctx) : stream_index_(index) {
    if (ctx == nullptr || index < 0 || index >= ctx->nb_streams) return;
    AVCodec *codec;
    int ret = 0;

    avctx_ = avcodec_alloc_context3(NULL);
    if (!avctx_) return;
    ret = avcodec_parameters_to_context(avctx_, ctx->streams[index]->codecpar);
    if (ret < 0) {
        avcodec_free_context(&avctx_);
        return;
    }
    av_codec_set_pkt_timebase(avctx_, ctx->streams[index]->time_base);
    codec = avcodec_find_decoder(avctx_->codec_id);
    avctx_->codec_id = codec->id;
    ret = avcodec_open2(avctx_, codec, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_FATAL, "Fail to open codec on stream %d\n", index);
        avcodec_free_context(&avctx_);
    }
    std::thread decode_thread(&Stream::decode, this);
    decode_thread.detach();
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
        ret = avcodec_send_packet(avctx_, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        {
            ELOG("stream %d avcodec_send_packet error %d", stream_index_, ret);
            continue;
        }
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avctx_, frame);
        if (ret < 0 && ret != AVERROR_EOF)
        {
            ELOG("stream %d avcodec_receive_frame error %d", stream_index_, ret);
            av_frame_unref(frame);
            continue;
        }

        frame->pts = av_frame_get_best_effort_timestamp(frame);
        PutFrame(frame);

    } while (true);

}

void Stream::PutFrame(AVFrame *frame) {
    std::unique_lock<std::mutex> lock(mtx_);
    frame_full_.wait(lock, [this] { return frame_queue_.size() < FRAME_MAX_SIZE; });
    frame_queue_.push(frame);
    frame_empty_.notify_one();
}

void Stream::GetFrame(AVFrame *frame) {
    std::unique_lock<std::mutex> lock(mtx_);
    frame_empty_.wait(lock, [this] { return !frame_queue_.empty();} );
    auto tmp_frame = frame_queue_.front();
    av_frame_move_ref(frame, tmp_frame);
    av_frame_unref(tmp_frame);
    frame_queue_.pop();
    frame_full_.notify_one();
}

AVCodecContext *Stream::GetAVCtx() const {
    return avctx_;
}

int Stream::GetIndex() const {
    return stream_index_;
}










