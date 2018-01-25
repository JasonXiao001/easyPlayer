//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/24.
//

#ifndef EASYPLAYER_STREAM_H
#define EASYPLAYER_STREAM_H

#include <mutex>
#include <condition_variable>
#include <queue>

extern "C" {
#include <libavcodec/avcodec.h>
#include "libavformat/avformat.h"
}

class Stream {
public:
    Stream(int index, const AVFormatContext *ctx);
    void PutPacket(AVPacket &pkt);

private:
    void decode();
    void GetPacket(AVPacket &pkt);

private:
    std::queue<AVPacket> packet_queue_;
    std::queue<AVFrame> frame_queue_;
    AVCodecContext *avctx;
    std::mutex mtx_;
    std::condition_variable packet_full_;
    std::condition_variable packet_empty_;
    std::condition_variable frame_full_;
    std::condition_variable frame_empty_;
    int stream_index_ = -1;
    const size_t PKT_MAX_SIZE = 8;
    const size_t FRAME_MAX_SIZE = 16;
};


#endif //EASYPLAYER_STREAM_H
