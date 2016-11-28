#include <jni.h>
#include <string>
#include <thread>
#include <mutex>
#include <android/log.h>
#include <opensles.h>

#include <android/native_window.h>
#include <android/native_window_jni.h>



#define LOGD(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "easyplayer-lib", format, ##__VA_ARGS__)

static JavaVM *gVm = NULL;
static jobject gObj = NULL;

static const int MEDIA_NOP = 0; // interface test message
static const int MEDIA_PREPARED = 1;
static const int MEDIA_PLAYBACK_COMPLETE = 2;
static const int MEDIA_BUFFERING_UPDATE = 3;
static const int MEDIA_SEEK_COMPLETE = 4;
static const int MEDIA_SET_VIDEO_SIZE = 5;
static const int MEDIA_TIMED_TEXT = 99;
static const int MEDIA_ERROR = 100;
static const int MEDIA_INFO = 200;

ANativeWindow* nativeWindow;
ANativeWindow_Buffer windowBuffer;
jmethodID gOnResolutionChange = NULL;
jmethodID gPostEventFromNative = NULL;
EasyPlayer easyPlayer;
EasyPlayer *mPlayer;


jint JNI_OnLoad (JavaVM *vm, void *reserved) {
    gVm = vm;
    return JNI_VERSION_1_4;
}


extern "C"
void
Java_cn_jx_easyplayerlib_player_EasyMediaPlayer__1nativeInit
        (JNIEnv *env, jobject obj) {
    gObj = env->NewGlobalRef(obj);
    if (gObj == NULL) return;
    jclass clazz = env->GetObjectClass(obj);
    if (NULL == gPostEventFromNative){
        gPostEventFromNative = env->GetMethodID(clazz,"postEventFromNative","(III)V");
        if (NULL == gOnResolutionChange){
            LOGD("Couldn't find postEventFromNative method.\n");
        }
    }
}


void test() {
    JNIEnv *env = NULL;
    if (0 == gVm->AttachCurrentThread(&env, NULL)) {
        env->CallVoidMethod(gObj, gPostEventFromNative, MEDIA_SET_VIDEO_SIZE, mPlayer->viddec.get_width(), mPlayer->viddec.get_height());
        gVm->DetachCurrentThread();
    }
}


void showPic() {
    mPlayer->wait_state(PlayerState::READY);
    if (!mPlayer->has_video()) return;
    if (0 > ANativeWindow_setBuffersGeometry(nativeWindow, mPlayer->viddec.get_width(), mPlayer->viddec.get_height(), WINDOW_FORMAT_RGBA_8888)){
        LOGD("Couldn't set buffers geometry.\n");
        ANativeWindow_release(nativeWindow);
        return;
    }
    LOGD("Start display.\n");
    test();
    AVFrame *frameRGBA = av_frame_alloc();
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, mPlayer->viddec.get_width(), mPlayer->viddec.get_height(),1);
    uint8_t *vOutBuffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, vOutBuffer, AV_PIX_FMT_RGBA, mPlayer->viddec.get_width(), mPlayer->viddec.get_height(), 1);
    while (mPlayer->get_img_frame(frameRGBA)) {
        if (ANativeWindow_lock(nativeWindow, &windowBuffer, NULL) < 0) {
            LOGD("cannot lock window");
        } else {
            uint8_t *dst = (uint8_t *) windowBuffer.bits;
            for (int h = 0; h < mPlayer->viddec.get_height(); h++)
            {
                memcpy(dst + h * windowBuffer.stride * 4,
                       vOutBuffer + h * frameRGBA->linesize[0],
                       frameRGBA->linesize[0]);
            }
            ANativeWindow_unlockAndPost(nativeWindow);

        }
    }

}

void playAudio() {
    mPlayer->wait_state(PlayerState::READY);
    createAudioEngine();
    createBufferQueueAudioPlayer(mPlayer->auddec.get_sample_rate(), mPlayer->auddec.get_channels());
    audioStart();
}



void log(void* ptr, int level, const char* fmt, va_list vl) {
    switch (level) {
        case AV_LOG_VERBOSE:
            __android_log_vprint(ANDROID_LOG_DEBUG,  "native-lib", fmt, vl);
            break;
        case AV_LOG_INFO:
            __android_log_vprint(ANDROID_LOG_INFO,  "native-lib", fmt, vl);
            break;
        case AV_LOG_WARNING:
            __android_log_vprint(ANDROID_LOG_WARN,  "native-lib", fmt, vl);
            break;
        case AV_LOG_ERROR:
            __android_log_vprint(ANDROID_LOG_ERROR,  "native-lib", fmt, vl);
            break;
        case AV_LOG_FATAL:
        case AV_LOG_PANIC:
            __android_log_vprint(ANDROID_LOG_FATAL,  "native-lib", fmt, vl);
            break;
        case AV_LOG_QUIET:
            __android_log_vprint(ANDROID_LOG_SILENT,  "native-lib", fmt, vl);
            break;
        default:
            break;
    }
}


extern "C"
void
Java_cn_jx_easyplayerlib_EasyPlayer_play
        (JNIEnv *env, jobject obj, jstring url, jobject surface) {

    char inputStr[500] = {0};
    sprintf(inputStr, "%s", env->GetStringUTFChars(url, NULL));
    av_log_set_callback(log);
    easyPlayer.init(inputStr);

//    player.init(inputStr);
    init(&easyPlayer);
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (0 == nativeWindow){
        LOGD("Couldn't get native window from surface.\n");
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
    std::thread videoThread(showPic);
    std::thread audioThread(playAudio);
    easyPlayer.wait_state(PlayerState::READY);
    if (easyPlayer.has_video()) {
        env->CallVoidMethod(obj, gOnResolutionChange, easyPlayer.viddec.get_width(), easyPlayer.viddec.get_height());
    }
//    std::thread decodeThread(decode);

    audioThread.join();
    videoThread.join();
//    decodeThread.join();

}


extern "C"
void
Java_cn_jx_easyplayer_MainActivity_togglePaused
        (JNIEnv *env, jobject obj) {
    easyPlayer.togglePaused();
}


//extern "C"
//void
//Java_cn_jx_easyplayer_MainActivity_togglePaused
//        (JNIEnv *env, jobject obj, int seek_pos) {
//    easyPlayer.stream_seek(seek_pos);
//}


extern "C"
void
Java_cn_jx_easyplayerlib_player_EasyMediaPlayer__1setDataSource
        (JNIEnv *env, jobject obj, jstring path) {
    mPlayer = new EasyPlayer();
    char inputStr[500] = {0};
    sprintf(inputStr, "%s", env->GetStringUTFChars(path, NULL));
    av_log_set_callback(log);
    mPlayer->set_data_source(inputStr);
    init(mPlayer);
}


extern "C"
void
Java_cn_jx_easyplayerlib_player_EasyMediaPlayer__1setVideoSurface
        (JNIEnv *env, jobject obj, jobject surface) {
    nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (0 == nativeWindow){
        LOGD("Couldn't get native window from surface.\n");
        return;
    }

}



extern "C"
void
Java_cn_jx_easyplayerlib_player_EasyMediaPlayer__1prepareAsync
        (JNIEnv *env, jobject obj) {
    mPlayer->prepare();
    std::thread videoThread(showPic);
    std::thread audioThread(playAudio);
    audioThread.join();
    videoThread.join();
}


extern "C"
void
Java_cn_jx_easyplayerlib_player_EasyMediaPlayer__1pause
        (JNIEnv *env, jobject obj) {
    if (!mPlayer->get_paused()) {
        mPlayer->togglePaused();
    }

}







