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


class Player : AudioDataSource{
public:
    static Player& Instance()
    {
        static Player instance;
        return instance;
    }
    void SetDataSource(const std::string &data_source);
    void Prepare();
    void PlayAudio();
    void SetupJNI(JNIEnv *env);
    bool GetAudioBuffer(int &nextSize, uint8_t *outputBuffer);
    Stream *GetAudioStream() const;

private:
    Player();
    ~Player();
    Player(const Player&);
    Player &operator=(const Player&);
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
    struct SwrContext *swr_ctx_;
    JNIEnv *env_;

};


#endif //EASYPLAYER_PLAYER_H
