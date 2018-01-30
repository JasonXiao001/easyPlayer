//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/23.
//

#ifndef EASYPLAYER_PLAYER_H
#define EASYPLAYER_PLAYER_H


#include <string>
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

#include "stream.h"
#include "audio_player.h"
#include "video_player.h"


class Player : AudioDataSource{
public:
    static Player& Instance()
    {
        static Player instance;
        return instance;
    }
    void SetDataSource(JNIEnv *env, const std::string &data_source);
    void CreateVideoPlayer(JNIEnv *env, jobject surface);
    void Prepare();
    void Start();

    void SetupJNI(JNIEnv *env);
    bool GetAudioBuffer(int &nextSize, uint8_t *outputBuffer);
    Stream *GetAudioStream() const;

private:
    Player();
    ~Player();
    Player(const Player&);
    Player &operator=(const Player&);
    void PlayAudio();
    void PlayVideo();
    void read();
    void release();
    virtual void GetAudioData(int &nextSize, uint8_t *outputBuffer) override;
    static void log(void* ptr, int level, const char* fmt, va_list vl);
private:
    std::string data_source_;
    AVFormatContext *ic_;
    Stream *audio_stream;
    Stream *video_stream;
    AudioPlayer audio_player;
    VideoPlayer *video_player;
    struct SwrContext *audio_swr_ctx_;
    struct SwsContext *video_swr_ctx_;
    JNIEnv *env_;

    bool stop_ = false;

};


#endif //EASYPLAYER_PLAYER_H
