//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/2/3.
//

#ifndef EASYPLAYER_EVENT_CALLBACK_H
#define EASYPLAYER_EVENT_CALLBACK_H

#include <jni.h>


class EventCallback {
public:
    virtual void OnPrepared() = 0;
    virtual ~EventCallback() = default;
};

class MyEventCallback : public EventCallback {
public:
    MyEventCallback(JNIEnv *env, jobject jcallback);
    virtual void OnPrepared() override;

private:
    JavaVM *g_VM;
    jobject cb_;
};


#endif //EASYPLAYER_EVENT_CALLBACK_H
