//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/27.
//

#include "audio_player.h"
#include "log_util.h"

AudioPlayer::AudioPlayer(AudioDataSource *data_source)
: engine_obj_(NULL)
, engine_(NULL)
, output_mix_obj_(NULL)
, buffer_queue_(NULL)
, audio_player_(NULL)
, buffer_size_(0)
, src_(data_source)
{

}

void AudioPlayer::ThrowException(JNIEnv *env, const char *class_name, const char *message) {
    jclass clazz = env->FindClass(class_name);
    if (0 != clazz)
    {
        env->ThrowNew(clazz, message);
        env->DeleteLocalRef(clazz);
    }
}

std::string AudioPlayer::ResultToString(SLresult result) {
    switch (result)
    {
        case SL_RESULT_SUCCESS:
            return "Success";
        case SL_RESULT_PRECONDITIONS_VIOLATED:
            return "Precondition violated";
        case SL_RESULT_PARAMETER_INVALID:
            return "Parameter invalid";
        case SL_RESULT_MEMORY_FAILURE:
            return "Memory failure";
        case SL_RESULT_RESOURCE_ERROR:
            return "Resource error";
        case SL_RESULT_RESOURCE_LOST:
            return "Resource lost";
        case SL_RESULT_IO_ERROR:
            return "IO error";
        case SL_RESULT_BUFFER_INSUFFICIENT:
            return "Buffer insufficient";
        case SL_RESULT_CONTENT_CORRUPTED:
            return "Success";
        case SL_RESULT_CONTENT_UNSUPPORTED:
            return "Content unsupported";
        case SL_RESULT_CONTENT_NOT_FOUND:
            return "Content not found";
        case SL_RESULT_PERMISSION_DENIED:
            return "Permission denied";
        case SL_RESULT_FEATURE_UNSUPPORTED:
            return "Feature unsupported";
        case SL_RESULT_INTERNAL_ERROR:
            return "Internal error";
        case SL_RESULT_UNKNOWN_ERROR:
            return "Unknown error";
        case SL_RESULT_OPERATION_ABORTED:
            return "Operation aborted";
        case SL_RESULT_CONTROL_LOST:
            return "Control lost";
        default:
            return "Unknown code";
    }
}

bool AudioPlayer::CheckError(JNIEnv *env, SLresult result) {
    if (SL_RESULT_SUCCESS != result)
    {
        auto ret_str = ResultToString(result).c_str();
//        ThrowException(env, "java/lang/IOException", ret_str);
        return true;
    }
    return false;
}

void AudioPlayer::CreateEngine(JNIEnv *env) {
    SLresult result;
    result = slCreateEngine(&engine_obj_, 0, NULL, 0, NULL, NULL);
    CheckError(env, result);
    result = (*engine_obj_)->Realize(engine_obj_, SL_BOOLEAN_FALSE);
    CheckError(env, result);
    result = (*engine_obj_)->GetInterface(engine_obj_, SL_IID_ENGINE, &engine_);
    CheckError(env, result);
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engine_)->CreateOutputMix(engine_, &output_mix_obj_, 1, ids, req);
    CheckError(env, result);
    result = (*output_mix_obj_)->Realize(output_mix_obj_, SL_BOOLEAN_FALSE);
    CheckError(env, result);
}

void AudioPlayer::CreateBufferQueuePlayer(JNIEnv *env, u_int32_t channel, u_int32_t sample_rate, u_int16_t sample_bits) {
    SLresult result;
    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, channel, sample_rate * 1000,
                                   SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};
    format_pcm.channelMask = channel == 2 ? SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT : SL_SPEAKER_FRONT_CENTER;
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, output_mix_obj_};
    SLDataSink audioSnk = {&loc_outmix, NULL};
    SLInterfaceID ids[] = { SL_IID_BUFFERQUEUE };
    SLboolean req[] = { SL_BOOLEAN_TRUE };
    result = (*engine_)->CreateAudioPlayer(engine_, &audio_player_obj_, &audioSrc, &audioSnk, 1, ids, req);
    CheckError(env, result);
    // realize the player
    result = (*audio_player_obj_)->Realize(audio_player_obj_, SL_BOOLEAN_FALSE);
    CheckError(env, result);
    // get the play interface
    result = (*audio_player_obj_)->GetInterface(audio_player_obj_, SL_IID_PLAY, &audio_player_);
    CheckError(env, result);
    // get the buffer queue interface
    buffer_size_ = channel * sample_rate * sample_bits;
    buffer_ = new unsigned char[buffer_size_];
    ILOG("create buffer with size %u", buffer_size_);
    result = (*audio_player_obj_)->GetInterface(audio_player_obj_, SL_IID_BUFFERQUEUE, &buffer_queue_);
    CheckError(env, result);
    result = (*buffer_queue_)->RegisterCallback(buffer_queue_, bqPlayerCallback, this);
    CheckError(env, result);
}

void AudioPlayer::Release() {

}

void AudioPlayer::bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    int next_size = 0;
    AudioPlayer *player = (AudioPlayer *)context;
    player->src_->GetAudioData(next_size, player->buffer_);
    if (0 != next_size) {
        // enqueue another buffer
        auto result = (*bq)->Enqueue(bq, player->buffer_, next_size);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT
    }
}

void AudioPlayer::Play() {
    auto result = (*audio_player_)->SetPlayState(audio_player_, SL_PLAYSTATE_PLAYING);
    bqPlayerCallback(buffer_queue_, this);
}

void AudioPlayer::Pause() {
    (*audio_player_)->SetPlayState(audio_player_, SL_PLAYSTATE_PAUSED);
}



















