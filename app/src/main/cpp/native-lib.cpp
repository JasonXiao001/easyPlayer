#include <jni.h>
#include <string>
#include <thread>
#include <mutex>
#include <android/log.h>
#include <player.h>

#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>


#define LOGD(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "native-lib", format, ##__VA_ARGS__)

ANativeWindow* nativeWindow;
ANativeWindow_Buffer windowBuffer;
Player player;
jmethodID gOnResolutionChange = NULL;

void showPic() {
    while (true) {
        AVFrame *frameRGBA = player.deQueuePic();
        if (frameRGBA == nullptr) break;
        if (ANativeWindow_lock(nativeWindow, &windowBuffer, NULL) < 0) {
            LOGD("cannot lock window");
        } else {
            uint8_t *dst = (uint8_t *) windowBuffer.bits;
            for (int h = 0; h < player.getHeight(); h++)
            {
                memcpy(dst + h * windowBuffer.stride * 4,
                       player.getOutBuffer() + h * frameRGBA->linesize[0],
                       frameRGBA->linesize[0]);
            }
            ANativeWindow_unlockAndPost(nativeWindow);

        }
        usleep(40000);
    }
}


extern "C"
void
Java_cn_jx_easyplayer_MainActivity_play
        (JNIEnv *env, jobject obj, jstring url, jobject surface) {
    char inputStr[500] = {0};
    sprintf(inputStr, "%s", env->GetStringUTFChars(url, NULL));
    player.init(inputStr);
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (0 == nativeWindow){
        LOGD("Couldn't get native window from surface.\n");
        return;
    }
    if (0 > ANativeWindow_setBuffersGeometry(nativeWindow, player.getWidth(), player.getHeight(), WINDOW_FORMAT_RGBA_8888)){
        LOGD("Couldn't set buffers geometry.\n");
        ANativeWindow_release(nativeWindow);
        return;
    }
    if (NULL == gOnResolutionChange){
        jclass clazz = env->GetObjectClass(obj);
        gOnResolutionChange = env->GetMethodID(clazz,"onResolutionChange","(II)V");
        if (NULL == gOnResolutionChange){
            LOGD("Couldn't find onResolutionChange method.\n");
            return;
        }
    }
    env->CallVoidMethod(obj, gOnResolutionChange, player.getWidth(), player.getHeight());
    player.decode();
    std::thread t(showPic);
    t.join();
}


