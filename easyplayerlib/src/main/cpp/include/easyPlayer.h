//
// Created by Administrator on 2016/11/10.
//

#ifndef EASYPLAYER_EASYPLAYER_H
#define EASYPLAYER_EASYPLAYER_H


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

#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <memory>
#include <unistd.h>

class PacketQueue {
public:
    int put_packet(AVPacket *pkt);
    int get_packet(AVPacket *pkt);
    int put_nullpacket();
    void set_abort(int abort);
    int get_abort();
    int get_serial();
    void flush();
    size_t get_queue_size();
private:
    std::queue<AVPacket> queue;
    int64_t duration;
    int abort_request = 1;
    int serial;
    std::mutex mutex;
    std::condition_variable cond;
    std::condition_variable full;
    const size_t MAX_SIZE = 8;

};


struct Frame {
    Frame(AVFrame *f) : frame(f){

    }
    AVFrame *frame;
    int serial;
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;          /* byte position of the frame in the input file */
    int allocated;
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
};


class FrameQueue {
public:
    void put_frame(AVFrame *frame);
    std::shared_ptr<Frame> get_frame();
    size_t get_size();
    int64_t frame_queue_last_pos();
private:
    std::queue<std::shared_ptr<Frame>> queue;
    std::mutex mutex;
    std::condition_variable empty;
    std::condition_variable full;
    const size_t MAX_SIZE = 16;
};

class Decoder {
public:
    virtual int decoder_decode_frame() = 0;
    virtual void decode() = 0;
    void init(AVCodecContext *ctx);
    void start_decode_thread();
    PacketQueue pkt_queue;
    FrameQueue frame_queue;
    AVCodecContext *avctx;
protected:
    AVPacket pkt;
    AVPacket pkt_temp;
    int pkt_serial;
    int finished;
    int packet_pending;
    std::condition_variable empty_queue_cond;
    int64_t start_pts;
    AVRational start_pts_tb;
    int64_t next_pts;
    AVRational next_pts_tb;
};


class VideoDecoder : public Decoder {
public:
    virtual int decoder_decode_frame() override ;
    virtual void decode() override ;
    int get_width();
    int get_height();
};

class AudioDecoder : public Decoder {
public:
    virtual int decoder_decode_frame() override ;
    virtual void decode() override ;
    int get_channels();
    int get_sample_rate();
private:

};


enum class PlayerState {
    UNKNOWN,
    INIT,
    BUFFERING,
    READY,
    PLAYING,
};


class EasyPlayer {
public:
    EasyPlayer() = default;
    void set_data_source(const std::string input_filename);
    void init(const std::string input_filename);
    void prepare();
    bool has_video();
    bool get_img_frame(AVFrame *frame);
    bool get_aud_buffer(int &nextSize, uint8_t *outputBuffer);
    void wait_state(PlayerState need_state);
    void wait_paused();
    void release();
    bool is_playing() {
        return state == PlayerState::PLAYING && !paused;
    }
    void play() {
        if (state == PlayerState::READY) {
            state = PlayerState::PLAYING;
            paused = false;
        }
        play_when_ready = true;
    }
    void pause() {
        av_log(NULL, AV_LOG_VERBOSE, "pause called,current state %d\n", state);
        if (state == PlayerState::PLAYING) {
            std::unique_lock<std::mutex> lock(mutex);
            paused = true;
            pause_condition.notify_all();
            state = PlayerState::READY;
        }

    }
    bool get_paused() {
        return paused;
    }
    int64_t get_duration() {
        if (ic != nullptr) {
            return ic->duration/1000;
        }
    };
    long get_curr_position() {
        return (long)audio_clock * 1000;
    };
    void stream_seek(int64_t pos);
    void set_event_listener(void (*cb)(int, int, int)) {
        event_listener = cb;
    }
    AVFormatContext *ic;
    char *filename;
    int abort_request;
    int force_refresh;

    int last_paused;
    int queue_attachments_req;
    bool seek_req = false;
    int seek_flags;
    int64_t seek_pos;
    int64_t seek_rel;
    int read_pause_return;
    bool realtime;
    AudioDecoder auddec;
    VideoDecoder viddec;
    PlayerState state = PlayerState::UNKNOWN;
private:
    void read();
    bool is_realtime();
    int stream_component_open(int stream_index);
    void on_state_change(PlayerState state);
    int last_video_stream, last_audio_stream;
    int video_stream = -1;
    AVStream *video_st;
    bool paused = false;
    bool play_when_ready = false;
    void (*event_listener)(int, int, int);

    double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    struct SwsContext *img_convert_ctx;

    double audio_clock;
    int audio_clock_serial;
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;

    int audio_hw_buf_size;
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audio_buf1_size;
    int audio_buf_index; /* in bytes */
    int audio_write_buf_size;
    int audio_volume;
    int muted;
    struct SwrContext *swr_ctx;
    int frame_drops_early;
    int frame_drops_late;
    int eof;

    int audio_stream = -1;
    int av_sync_type;

    int64_t start_time = AV_NOPTS_VALUE;
    int64_t duration = AV_NOPTS_VALUE;
    std::mutex mutex;
    std::condition_variable state_condition;
    std::condition_variable pause_condition;


    static const int MEDIA_NOP = 0; // interface test message
    static const int MEDIA_PREPARED = 1;
    static const int MEDIA_PLAYBACK_COMPLETE = 2;
    static const int MEDIA_BUFFERING_UPDATE = 3;
    static const int MEDIA_SEEK_COMPLETE = 4;
    static const int MEDIA_SET_VIDEO_SIZE = 5;
    static const int MEDIA_TIMED_TEXT = 99;
    static const int MEDIA_ERROR = 100;
    static const int MEDIA_INFO = 200;

};


#endif //EASYPLAYER_EASYPLAYER_H
