#include <jni.h>

#include "log_util.h"
#include "easy_player.h"

#include <android/native_window.h>
#include <android/native_window_jni.h>


EasyPlayer *player = new EasyPlayer();


extern "C"
void
Java_cn_jx_easyplayer_MainActivity_togglePaused
        (JNIEnv *env, jobject obj) {
}


extern "C"
void
Java_cn_jx_easyplayer_MainActivity__1setDataSource
        (JNIEnv *env, jobject obj, jstring url) {
    player->SetDataSource(env->GetStringUTFChars(url, NULL));
    player->PrepareAsync();
}




