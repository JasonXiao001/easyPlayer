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
#include "opensles.h"
#include "event_callback.h"
#include "native_window_renderer.h"
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

#define AUDIO_BUFFER_SIZE 8196
#define AUDIO_READY_SIZE 8

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
    std::condition_variable full_;
    const size_t kMaxSize = 16;
};

class FrameQueue {
public:
    void Put(AVFrame *frame);
    AVFrame *Get();
    size_t Size() const {
        return queue_.size();
    };
private:
    std::queue<AVFrame*> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    const size_t kMaxSize = 16;
    const size_t kReadySize = 8;
};



class EasyPlayer : public AudioDataProvider, public VideoDataProvider {
public:
    EasyPlayer();
    EasyPlayer(const EasyPlayer&) = delete;
    ~EasyPlayer();
    int SetDataSource(const std::string &path);
    int PrepareAsync();
    void SetEventCallback(EventCallback *cb);
    int Start();
    int Pause();
    virtual void GetData(uint8_t **buffer, int &buffer_size) override;
    virtual void GetData(uint8_t **buffer, AVFrame **frame, int &width, int &height) override;
    virtual int GetVideoWidth() override;
    virtual int GetVideoHeight() override;

private:
    void read();
    void decodeAudio();
    void decodeVideo();
    void openStream(int index);
private:
    EventCallback *event_cb_;
    State state_;
    std::unique_ptr<std::thread> read_thread_;
    std::unique_ptr<std::thread> audio_decode_thread_;
    std::unique_ptr<std::thread> video_decode_thread_;
    std::unique_ptr<std::thread> audio_render_thread_;
    std::unique_ptr<std::thread> video_render_thread_;
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
    uint8_t *audio_buffer_;
    AVFrame *frame_rgba_;
    uint8_t *rgba_buffer_;
    double audio_clock;
};


#endif //EASYPLAYER_EASY_PLAYER_H
