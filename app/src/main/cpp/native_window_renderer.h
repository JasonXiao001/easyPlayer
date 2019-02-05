//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/2/5.
//

#ifndef EASYPLAYER_NATIVE_WINDOW_RENDERER_H
#define EASYPLAYER_NATIVE_WINDOW_RENDERER_H

#include <jni.h>
extern "C"{
#include "libavformat/avformat.h"
};

class VideoDataProvider {
public:
    virtual void GetData(uint8_t **buffer, AVFrame **frame, int &width, int &height) = 0;
    virtual int GetVideoWidth() = 0;
    virtual int GetVideoHeight() = 0;
    virtual ~VideoDataProvider() = default;
};

void initVideoRenderer(JNIEnv *env, VideoDataProvider *provider, jobject surface);
void videoRender();

#endif //EASYPLAYER_NATIVE_WINDOW_RENDERER_H
