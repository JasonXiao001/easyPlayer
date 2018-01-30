//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/23.
//

#include <thread>
#include "player.h"
#include "log_util.h"




void Player::SetDataSource(JNIEnv *env, const std::string &data_source) {
    ILOG("set data source %s", data_source.c_str());
    data_source_ = data_source;
    env_ = env;

}

Player::Player() : audio_player(this) {
    av_register_all();
    avformat_network_init();
//    av_log_set_callback(log);
//    av_log_set_level(AV_LOG_INFO);
}

Player::~Player() {
}

void Player::Prepare() {
    int err, i;
    int st_index[AVMEDIA_TYPE_NB];
    memset(st_index, -1, sizeof(st_index));
    ic_ = avformat_alloc_context();
    if (!ic_) {
        ELOG("Could not allocate context.");
        return;
    }
    err = avformat_open_input(&ic_, data_source_.c_str(), NULL, NULL);
    if (err < 0) {
        ELOG("Could not open input file.");
        release();
    }
    err = avformat_find_stream_info(ic_, NULL);
    if (err < 0) {
        av_log(NULL, AV_LOG_WARNING,
               "%s: could not find codec parameters\n", data_source_.c_str());
        release();
    }

    for(i = 0; i < ic_->nb_streams; i++) {
        if (ic_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            st_index[AVMEDIA_TYPE_AUDIO] = i;
            ILOG("start open audio component at id %d.\n", st_index[AVMEDIA_TYPE_AUDIO]);
            audio_stream = new Stream(i, ic_);
            audio_swr_ctx_ = swr_alloc();
            audio_swr_ctx_ = swr_alloc_set_opts(NULL,
                                          audio_stream->GetAVCtx()->channel_layout, AV_SAMPLE_FMT_S16, audio_stream->GetAVCtx()->sample_rate,
                                          audio_stream->GetAVCtx()->channel_layout, audio_stream->GetAVCtx()->sample_fmt, audio_stream->GetAVCtx()->sample_rate,
                                          0, NULL);
            if (!audio_swr_ctx_ || swr_init(audio_swr_ctx_) < 0) {
                ELOG("Cannot create sample rate converter for conversion channels!\n");
                swr_free(&audio_swr_ctx_);
                return;
            }
            audio_player.CreateEngine(env_);
            audio_player.CreateBufferQueuePlayer(env_, audio_stream->GetAVCtx()->channels, audio_stream->GetAVCtx()->sample_rate, audio_stream->GetAVCtx()->bits_per_raw_sample);
        }
        if (ic_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            st_index[AVMEDIA_TYPE_VIDEO] = i;
            video_stream = new Stream(i, ic_);
            auto avctx = video_stream->GetAVCtx();
            video_swr_ctx_ = sws_getContext(avctx->width, avctx->height, avctx->pix_fmt,
                                             avctx->width, avctx->height, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
            video_player->Setup(avctx->width, avctx->height);
        }
    }
    if (video_stream == nullptr && audio_stream == nullptr) {
        av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n", data_source_.c_str());
        release();
    }
    std::thread read_thread(&Player::read, this);
    read_thread.detach();
}

void Player::read() {
    AVPacket *pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
    int ret = 0;
    while(true) {
        DLOG("read one");
        ret = av_read_frame(ic_, pkt);
        if (ret < 0) {
            DLOG("read one %d", ret);
            break;
        }
        /* check if packet is in play range specified by user, then queue, otherwise discard */

        if (pkt->stream_index == audio_stream->GetIndex()) {
            DLOG("read audio");
            audio_stream->PutPacket(*pkt);
        }else if (pkt->stream_index == video_stream->GetIndex()) {
            DLOG("read video");
            video_stream->PutPacket(*pkt);
        }
    }
}

void Player::release() {
    stop_ = true;
}

bool Player::GetAudioBuffer(int &nextSize, uint8_t *outputBuffer) {
    if (outputBuffer == nullptr) return false;
    auto frame = av_frame_alloc();
    audio_stream->GetFrame(frame);
    av_log(NULL, AV_LOG_INFO, "GetAudioBuffer %d", frame->channels);
    auto ctx = audio_stream->GetAVCtx();
    if (ctx->sample_fmt == AV_SAMPLE_FMT_S16P) {
        nextSize = av_samples_get_buffer_size(frame->linesize, ctx->channels, ctx->frame_size, ctx->sample_fmt, 1);
    }else {
        av_samples_get_buffer_size(&nextSize, ctx->channels, ctx->frame_size, ctx->sample_fmt, 1);
    }
    int ret = swr_convert(audio_swr_ctx_, &outputBuffer, frame->nb_samples,
                          (uint8_t const **) (frame->extended_data),
                          frame->nb_samples);
    av_frame_unref(frame);
    return ret >= 0;
}

Stream *Player::GetAudioStream() const {
    return audio_stream;
}



void Player::log(void *ptr, int level, const char *fmt, va_list vl) {
    int lvl = LOG_LEVEL_V;
    switch (level) {
        case AV_LOG_VERBOSE:
            lvl = LOG_LEVEL_V;
            break;
        case AV_LOG_DEBUG:
            lvl = LOG_LEVEL_D;
            break;
        case AV_LOG_INFO:
            lvl = LOG_LEVEL_I;
            break;
        case AV_LOG_WARNING:
            lvl = LOG_LEVEL_W;
            break;
        case AV_LOG_ERROR:
        case AV_LOG_FATAL:
        case AV_LOG_PANIC:
            lvl = LOG_LEVEL_E;
            break;
        case AV_LOG_QUIET:
            lvl = LOG_LEVEL_N;
            break;
        default:
            break;
    }
    LogUtil::LogVl(lvl, "ffmpeg", fmt, vl);
}

void Player::GetAudioData(int &nextSize, uint8_t *outputBuffer) {
    if (outputBuffer == nullptr) return;
    auto frame = av_frame_alloc();
    audio_stream->GetFrame(frame);
    av_log(NULL, AV_LOG_INFO, "GetAudioBuffer %d", frame->channels);
    auto ctx = audio_stream->GetAVCtx();
    if (ctx->sample_fmt == AV_SAMPLE_FMT_S16P) {
        nextSize = av_samples_get_buffer_size(frame->linesize, ctx->channels, ctx->frame_size, ctx->sample_fmt, 1);
    }else {
        av_samples_get_buffer_size(&nextSize, ctx->channels, ctx->frame_size, ctx->sample_fmt, 1);
    }
    swr_convert(audio_swr_ctx_, &outputBuffer, frame->nb_samples,
                          (uint8_t const **) (frame->extended_data),
                          frame->nb_samples);
    av_frame_unref(frame);
}

void Player::SetupJNI(JNIEnv *env) {
    env_ = env;
}

void Player::PlayAudio() {
    audio_player.Play();
}

void Player::Start() {
    DLOG("start play");
    if (audio_stream != nullptr) {
        std::thread audio_thread(&Player::PlayAudio, this);
        audio_thread.detach();
    }
    if (video_stream != nullptr && video_player != nullptr) {
        std::thread video_thread(&Player::PlayVideo, this);
        video_thread.detach();
    }
}

void Player::PlayVideo() {
    DLOG("play video");
    AVFrame *frameRGBA = av_frame_alloc();
    int bytes_num = av_image_get_buffer_size(AV_PIX_FMT_RGBA, video_stream->GetAVCtx()->width, video_stream->GetAVCtx()->height, 1);
    uint8_t *out_buffer = (uint8_t *)av_malloc(bytes_num*sizeof(uint8_t));
    av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, out_buffer, AV_PIX_FMT_RGBA, video_stream->GetAVCtx()->width, video_stream->GetAVCtx()->height, 1);
    while (!stop_) {
        video_stream->GetFrame(frameRGBA);
        DLOG("play while");
        sws_scale(video_swr_ctx_, (const uint8_t* const*)frameRGBA->data, frameRGBA->linesize, 0, video_stream->GetAVCtx()->height,
                  frameRGBA->data, frameRGBA->linesize);
        video_player->Show(out_buffer, frameRGBA->linesize[0]);
    }
    av_frame_unref(frameRGBA);
    av_frame_free(&frameRGBA);
}

void Player::CreateVideoPlayer(JNIEnv *env, jobject surface) {
    if (video_player != nullptr) {
        delete video_player;
    }
    video_player = new VideoPlayer(env, surface);
}


























