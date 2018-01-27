//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/23.
//

#include <thread>
#include "player.h"




void Player::SetDataSource(const std::string &data_source) {
    data_source_ = data_source;

}

Player::Player() {
    av_register_all();
    avformat_network_init();
}

Player::~Player() {
}

void Player::Prepare() {

    int err, i;
    int st_index[AVMEDIA_TYPE_NB];
    memset(st_index, -1, sizeof(st_index));
    ic_ = avformat_alloc_context();
    if (!ic_) {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate context.\n");
        return;
    }
    err = avformat_open_input(&ic_, data_source_.c_str(), NULL, NULL);
    if (err < 0) {
        av_log(NULL, AV_LOG_FATAL, "Could not open input file.\n");
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
            av_log(NULL, AV_LOG_INFO, "start open audio component at id %d.\n",st_index[AVMEDIA_TYPE_AUDIO]);

            audio_stream = new Stream(i, ic_);
            swr_ctx_ = swr_alloc();
            swr_ctx_ = swr_alloc_set_opts(NULL,
                                          audio_stream->GetAVCtx()->channel_layout, AV_SAMPLE_FMT_S16, audio_stream->GetAVCtx()->sample_rate,
                                          audio_stream->GetAVCtx()->channel_layout, audio_stream->GetAVCtx()->sample_fmt, audio_stream->GetAVCtx()->sample_rate,
                                          0, NULL);
            if (!swr_ctx_ || swr_init(swr_ctx_) < 0) {
                av_log(NULL, AV_LOG_ERROR,
                       "Cannot create sample rate converter for conversion channels!\n");
                swr_free(&swr_ctx_);
                return;
            }
        }
//        if (ic_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
//            st_index[AVMEDIA_TYPE_VIDEO] = i;
//            av_log(NULL, AV_LOG_INFO, "start open video component at id %d.\n",st_index[AVMEDIA_TYPE_VIDEO]);
//            video_stream = new Stream(i, ic_);
//        }
    }
    if (video_stream < 0 && audio_stream < 0) {
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

        ret = av_read_frame(ic_, pkt);
        if (ret < 0) {
            break;
        }
        /* check if packet is in play range specified by user, then queue, otherwise discard */

        if (pkt->stream_index == audio_stream->GetIndex()) {
            audio_stream->PutPacket(*pkt);
        }
    }
}

void Player::release() {

}

bool Player::GetAudioBuffer(int &nextSize, uint8_t *outputBuffer) {
    av_log(NULL, AV_LOG_INFO, "here !1111111");
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
    int ret = swr_convert(swr_ctx_, &outputBuffer, frame->nb_samples,
                          (uint8_t const **) (frame->extended_data),
                          frame->nb_samples);
    av_frame_unref(frame);
    return ret >= 0;
}

Stream *Player::GetAudioStream() const {
    return audio_stream;
}

Player::Player(const Player &) {

}









