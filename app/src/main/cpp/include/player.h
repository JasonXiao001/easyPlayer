//
// Created by JasonXiao on 2016/10/24.
//

#ifndef EASYPLAYER_PLAYER_H
#define EASYPLAYER_PLAYER_H

#include <string>
#include <assert.h>
#include <queue>
#include <mutex>
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
};


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
    AVFrame *deQueuePic();
    void release();
private:
    std::string url;
    std::mutex queueMutex;
    std::queue<AVPacket> picQueue;
    int audioIndex = -1;
    int videoIndex = -1;
    int width, height;
    AVFormatContext *pFormatCtx;
    AVCodecContext *aCodecCtx, *vCodecCtx;
    AVCodec *aCodec, *vCodec;
    AVFrame *aFrame, vFrame, *frameRGBA;
    SwsContext *imgConvertCtx;
    uint8_t *vOutBuffer;
    void enQueue(AVPacket *packet);

};


#endif //EASYPLAYER_PLAYER_H
