#include <jni.h>

#include "log_util.h"
#include "easy_player.h"
#include "event_callback.h"

EasyPlayer *player = new EasyPlayer();
MyEventCallback *eventCallback;



extern "C"
void
Java_cn_jx_easyplayer_EasyPlayer__1setEventCallback
        (JNIEnv *env, jobject obj, jobject jcallback) {
    if (eventCallback) {
        delete eventCallback;
    }
    eventCallback = new MyEventCallback(env, jcallback);
    player->SetEventCallback(eventCallback);
}



extern "C"
void
Java_cn_jx_easyplayer_EasyPlayer__1setDataSource
        (JNIEnv *env, jobject obj, jstring url) {
    player->SetDataSource(env->GetStringUTFChars(url, NULL));
}


extern "C"
void
Java_cn_jx_easyplayer_EasyPlayer__1prepareAsync
        (JNIEnv *env, jobject obj) {
    player->PrepareAsync();
}


extern "C"
void
Java_cn_jx_easyplayer_EasyPlayer__1start
        (JNIEnv *env, jobject obj) {
    player->Start();
}


extern "C"
void
Java_cn_jx_easyplayer_EasyPlayer__1pause
        (JNIEnv *env, jobject obj) {
    player->Pause();
}

extern "C"
void
Java_cn_jx_easyplayer_EasyPlayer__1setSurface
        (JNIEnv *env, jobject obj, jobject surface) {
    initVideoRenderer(env, player, surface);
}

extern "C"
jint
Java_cn_jx_easyplayer_EasyPlayer__1getVideoWidth
        (JNIEnv *env, jobject obj) {
    return player->GetVideoWidth();
}

extern "C"
jint
Java_cn_jx_easyplayer_EasyPlayer__1getVideoHeight
        (JNIEnv *env, jobject obj) {
    return player->GetVideoHeight();
}




