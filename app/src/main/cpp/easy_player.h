//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/1/19.
//

#ifndef EASYPLAYER_EASY_PLAYER_H
#define EASYPLAYER_EASY_PLAYER_H

#include <thread>
#include <string>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <queue>
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libavcodec/avfft.h"
};

enum class State {
    Idle,
    Initialized,
    Preparing,
    Prepared,
    Started,
    Paused,
    Stoped,
    Completed,
    End,
    Error,
};

class PacketQueue {
public:
    void Put(AVPacket *pkt);
    void Get(AVPacket *pkt);
    void Clear();
    size_t Size();
private:
    std::queue<AVPacket> queue_;
    int64_t duration_;
    std::mutex mutex_;
    std::condition_variable ready_;
    const size_t kMaxSize = 16;
};

class FrameQueue {
public:
    void Put(AVFrame *frame);
    AVFrame *Get();

private:
    std::queue<AVFrame*> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    const size_t kMaxSize = 16;
    const size_t kReadySize = 1;
};


class EasyPlayer {
public:
    EasyPlayer();
    EasyPlayer(const EasyPlayer&) = delete;
    ~EasyPlayer();
    int SetDataSource(const std::string &path);
    int PrepareAsync();

private:
    void read();
    void decodeAudio();
    void decodeVideo();
    void openStream(int index);
private:
    State state_;
    std::unique_ptr<std::thread> read_thread_;
    std::unique_ptr<std::thread> audio_decode_thread_;
    std::unique_ptr<std::thread> video_decode_thread_;
    AVFormatContext *ic_;
    std::string path_;
    PacketQueue audio_packets_;
    PacketQueue video_packets_;
    FrameQueue audio_frames_;
    FrameQueue video_frames_;
    int video_stream_;
    int audio_stream_;
    AVStream *audio_st_;
    AVStream *video_st_;
    struct SwrContext *swr_ctx_;
    struct SwsContext *img_convert_ctx_;
    AVCodecContext *audio_codec_ctx_;
    AVCodecContext *video_codec_ctx_;
    int eof;
};


#endif //EASYPLAYER_EASY_PLAYER_H
