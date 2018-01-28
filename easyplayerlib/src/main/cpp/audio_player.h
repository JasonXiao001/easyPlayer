//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/27.
//

#ifndef EASYPLAYER_AUDIO_PLAYER_H
#define EASYPLAYER_AUDIO_PLAYER_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <stddef.h>
#include <jni.h>
#include <string>

class AudioDataSource
{
public:
    virtual void GetAudioData(int &nextSize, uint8_t *outputBuffer) = 0;
};

class AudioPlayer {
public:
    AudioPlayer(AudioDataSource *data_source);
    void CreateEngine(JNIEnv *env);
    void CreateBufferQueuePlayer(JNIEnv *env, u_int32_t channel, u_int32_t sample_rate, u_int16_t sample_bits);
    void Play();
    void Pause();
    void Release();

private:
    static void ThrowException(JNIEnv *env, const char *class_name, const char *message);
    static std::string ResultToString(SLresult result);
    static bool CheckError(JNIEnv *env, SLresult result);
    static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

private:
    SLObjectItf engine_obj_;
    SLEngineItf engine_;
    SLObjectItf output_mix_obj_;
    SLObjectItf audio_player_obj_;
    SLAndroidSimpleBufferQueueItf buffer_queue_;
    SLPlayItf audio_player_;

    unsigned char *buffer_;
    size_t buffer_size_;
    AudioDataSource *src_;

};


#endif //EASYPLAYER_AUDIO_PLAYER_H
