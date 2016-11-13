//
// Created by JasonXiao on 2016/10/24.
//

#ifndef EASYPLAYER_PLAYER_H
#define EASYPLAYER_PLAYER_H

#include <string>
#include <assert.h>
#include <queue>
#include <mutex>
#include <android/log.h>
#include <unistd.h>
#include "clock.h"

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
};



/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0


class Player {

public:
    Player() = default;
    void init(const std::string url);
    void decode();
    int getWidth() {
        return width;
    }
    int getHeight() {
        return height;
    }
    uint8_t *getOutBuffer(){
        return vOutBuffer;
    }
    int getSampleRate() {
        if (!aCodecCtx) return 0;
        return aCodecCtx->sample_rate;
    }
    int getChannel() {
        if (!aCodecCtx) return 0;
        return aCodecCtx->channels;
    }
    bool isVideo() {
        return videoIndex >= 0;
    }
    AVFrame *deQueuePic();
    void deQueueAudio(int &nextSize, uint8_t *outputBuffer);
    void start();
    void release();
private:
    std::string url;
    std::mutex queueMutex;
    std::queue<AVPacket> picQueue;
    std::queue<AVPacket> audioQueue;
    int audioIndex = -1;
    int videoIndex = -1;
    int width, height;
    AVFormatContext *pFormatCtx;
    AVCodecContext *aCodecCtx, *vCodecCtx;
    AVStream *aStream, *vStream;
    AVCodec *aCodec, *vCodec;
    AVFrame aFrame, vFrame, *frameRGBA;
    SwsContext *imgConvertCtx;
    SwrContext *audioConvertCtx;
    uint8_t *vOutBuffer;
//    Clock clock;
    double audioClock;

    void enQueue(AVPacket *packet);

};


#endif //EASYPLAYER_PLAYER_H
