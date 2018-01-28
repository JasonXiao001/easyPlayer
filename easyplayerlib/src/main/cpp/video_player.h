//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/28.
//

#ifndef EASYPLAYER_VIDEO_PLAYER_H
#define EASYPLAYER_VIDEO_PLAYER_H

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>


class VideoPlayer {
public:
    VideoPlayer(JNIEnv *env, jobject surface);
    void Setup(int32_t width, int32_t height, int32_t format);
    void Show(uint8_t *out_buffer, int line_size);

private:
    ~VideoPlayer();

private:
    ANativeWindow* nativeWindow;
    ANativeWindow_Buffer windowBuffer;
    uint32_t width_;
    uint32_t height_;

};


#endif //EASYPLAYER_VIDEO_PLAYER_H
