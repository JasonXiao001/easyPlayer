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
    void PutPacket(AVPacket *pkt);

private:
    std::queue<AVPacket> queue_;
    AVCodecContext *avctx;
    std::mutex mtx_;
    std::condition_variable full_;
    int stream_index_ = -1;
    const size_t MAX_SIZE = 8;
};


#endif //EASYPLAYER_STREAM_H
