//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/2/5.
//

#include "native_window_renderer.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "log_util.h"
#include <string>

ANativeWindow* nativeWindow;
ANativeWindow_Buffer windowBuffer;
VideoDataProvider *dataProvider = nullptr;

void initVideoRenderer(JNIEnv *env, VideoDataProvider *provider, jobject surface) {
    dataProvider = provider;
    nativeWindow = ANativeWindow_fromSurface(env, surface);
}

void videoRender() {
    if (dataProvider == nullptr) {
        ELOG("video render not init");
        return;
    }
    if (0 > ANativeWindow_setBuffersGeometry(nativeWindow, dataProvider->GetVideoWidth(), dataProvider->GetVideoHeight(), WINDOW_FORMAT_RGBA_8888)){
        ELOG("Couldn't set buffers geometry.\n");
        ANativeWindow_release(nativeWindow);
        return;
    }
    while (true) {
        if (ANativeWindow_lock(nativeWindow, &windowBuffer, nullptr) < 0) {
            ELOG("cannot lock window");
        } else {
            uint8_t *buffer = nullptr;
            AVFrame *frame = nullptr;
            int width, height;
            dataProvider->GetData(&buffer, &frame, width, height);
            if (buffer == nullptr) {
                break;
            }
            uint8_t *dst = (uint8_t *) windowBuffer.bits;
            for (int h = 0; h < height; h++)
            {
                memcpy(dst + h * windowBuffer.stride * 4,
                       buffer + h * frame->linesize[0],
                       frame->linesize[0]);
            }
            ANativeWindow_unlockAndPost(nativeWindow);
        }
    }
}