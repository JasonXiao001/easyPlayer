//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/2/3.
//

#include "event_callback.h"
#include "log_util.h"

void MyEventCallback::OnPrepared() {
    JNIEnv *env;
    bool need_detach = false;
    int getEnvStat = g_VM->GetEnv((void **) &env,JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        if (g_VM->AttachCurrentThread(&env, nullptr) != 0) {
            return;
        }
        need_detach = true;
    }
    jclass javaClass = env->GetObjectClass(cb_);
    if (javaClass == 0) {
        ELOG("Unable to find class");
        g_VM->DetachCurrentThread();
        return;
    }
    jmethodID javaCallbackId = env->GetMethodID(javaClass, "onPrepared", "()V");
    if (javaCallbackId == nullptr) {
        ELOG("Unable to find method:onPrepared");
        return;
    }
    env->CallVoidMethod(cb_, javaCallbackId);
    if(need_detach) {
        g_VM->DetachCurrentThread();
    }
    env = nullptr;
}

MyEventCallback::MyEventCallback(JNIEnv *env, jobject jcallback) {
    env->GetJavaVM(&g_VM);
    cb_ = env->NewGlobalRef(jcallback);
}
