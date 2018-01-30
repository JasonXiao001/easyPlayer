//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/28.
//

#include <string.h>
#include "video_player.h"
#include "log_util.h"

VideoPlayer::VideoPlayer(JNIEnv *env, jobject surface) {
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (0 == nativeWindow) {
        ELOG("Couldn't get native window from surface.\n");
    }
}

VideoPlayer::~VideoPlayer() {
    if (nativeWindow != nullptr) {
        ANativeWindow_release(nativeWindow);
    }
}

void VideoPlayer::Setup(int32_t width, int32_t height, int32_t format) {
    if (nativeWindow != nullptr) {
        auto ret = ANativeWindow_setBuffersGeometry(nativeWindow, width, height, format);
        height_ = height;
        width_ = width;
        if (ret != 0) {
            ELOG("set video player error");
        }

    }
}

void VideoPlayer::Show(uint8_t *out_buffer, int line_size) {
    if (ANativeWindow_lock(nativeWindow, &windowBuffer, NULL) < 0) {
        ELOG("cannot lock window");
    } else {
        uint8_t *dst = (uint8_t *) windowBuffer.bits;
        for (int h = 0; h < height_; h++)
        {
            memcpy(dst + h * windowBuffer.stride * 4,
                   out_buffer + h * line_size,
                   line_size);
        }
        ANativeWindow_unlockAndPost(nativeWindow);
    }
}










