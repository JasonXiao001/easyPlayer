//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/1/19.
//

#include "easy_player.h"
#include "log_util.h"
#include "common.h"
#include "opensles.h"


void EasyAudioPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    EasyPlayer *easyPlayer = (EasyPlayer *)context;
    easyPlayer->EnqueueAudioBuffer(bq);
}



void PacketQueue::Put(AVPacket *pkt) {
    std::lock_guard<std::mutex> lck(mutex_);
    if (queue_.size() >= kMaxSize) {
        WLOG("packet queue is full");
        auto dump_pkt = queue_.front();
        av_packet_unref(&dump_pkt);
        queue_.pop();
    }
    queue_.push(*pkt);
    ready_.notify_all();
}

void PacketQueue::Get(AVPacket *pkt) {
    if (pkt == nullptr) return;
    std::unique_lock<std::mutex> lck(mutex_);
    ready_.wait(lck, [this]{
        return !queue_.empty();
    });
    *pkt = queue_.front();
    queue_.pop();
}

void PacketQueue::Clear() {
    std::lock_guard<std::mutex> lck(mutex_);

}

AVFrame *FrameQueue::Get() {
    std::unique_lock<std::mutex> lck(mutex_);
    cond_.wait(lck, [this]{
        return !queue_.empty();
    });
    AVFrame *frame = queue_.front();
    queue_.pop();
    return frame;
}

void FrameQueue::Put(AVFrame *frame) {
    std::lock_guard<std::mutex> lck(mutex_);
    AVFrame *tmp = av_frame_alloc();
    av_frame_move_ref(tmp, frame);
    queue_.push(tmp);
    cond_.notify_all();
}


EasyPlayer::EasyPlayer() : state_(State::Idle),
                           ic_(nullptr), eof(0), video_stream_(-1), audio_stream_(-1) {
    av_log_set_callback(ff_log_callback);
    av_log_set_level(AV_LOG_DEBUG);
    av_register_all();
    avformat_network_init();
}

EasyPlayer::~EasyPlayer() {
    if (ic_) {
        avformat_free_context(ic_);
        ic_ = nullptr;
    }
}

int EasyPlayer::SetDataSource(const std::string &path) {
    if (state_ != State::Idle) {
        ELOG("illegal state|current:%d", state_);
        return ERROR_ILLEGAL_STATE;
    }
    ILOG("source path:%s", path.c_str());
    path_ = path;
    state_ = State ::Initialized;
    return SUCCESS;
}

int EasyPlayer::PrepareAsync() {
    if (state_ != State::Initialized && state_ != State::Stoped) {
        ELOG("illegal state|current:%d", state_);
        return ERROR_ILLEGAL_STATE;
    }
    read_thread_.reset(new std::thread(&EasyPlayer::read, this));
    return SUCCESS;
}

void EasyPlayer::EnqueueAudioBuffer(SLAndroidSimpleBufferQueueItf bq) {
    if (audio_buffer_ == nullptr) return;
    auto frame = audio_frames_.Get();
    int next_size;
    if (audio_codec_ctx_->sample_fmt == AV_SAMPLE_FMT_S16P) {
        next_size = av_samples_get_buffer_size(frame->linesize, audio_codec_ctx_->channels, audio_codec_ctx_->frame_size, audio_codec_ctx_->sample_fmt, 1);
    }else {
        av_samples_get_buffer_size(&next_size, audio_codec_ctx_->channels, audio_codec_ctx_->frame_size, audio_codec_ctx_->sample_fmt, 1);
    }
    int ret = swr_convert(swr_ctx_, &audio_buffer_, frame->nb_samples,
                          (uint8_t const **) (frame->extended_data),
                          frame->nb_samples);
    av_frame_unref(frame);
    av_frame_free(&frame);
    if (0 != next_size) {
        SLresult result;
        result = (*bq)->Enqueue(bq, audio_buffer_, next_size);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        if (SL_RESULT_SUCCESS != result) {
            ELOG("EnqueueAudioBuffer error");
//            pthread_mutex_unlock(&audioEngineLock);
        }
        (void)result;
    }
}

void EasyPlayer::read() {
    int err;
    char err_buff[1024];
    err = avformat_open_input(&ic_, path_.c_str(), NULL, NULL);
    if (err) {
        av_strerror(err, err_buff, sizeof(err_buff));
        ELOG("avformat_open_input failed|ret:%d|msg:%s", err, err_buff);
        return;
    }
    err = avformat_find_stream_info(ic_, NULL);
    if (err < 0) {
        ELOG("could not find codec parameters");
        return;
    }
    for(int i = 0; i < ic_->nb_streams; i++) {
        if (ic_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_ = i;
            ILOG("find audio stream index %d", i);
        }
        if (ic_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_ = i;
            ILOG("find video stream index %d", i);
        }
    }
    // start audio decode thread
    if (audio_stream_ >= 0) {
        openStream(audio_stream_);
    }
    // start video decode thread
    if (video_stream_ > 0) {
        openStream(video_stream_);
    }

    AVPacket *pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
    if (pkt == NULL) {
        ELOG("Could not allocate avPacket");
        return;
    }
    while (true) {
        err = av_read_frame(ic_, pkt);
        if (err < 0) {
            if ((err == AVERROR_EOF || avio_feof(ic_->pb))) {
                eof = 1;
            }
            if (ic_->pb && ic_->pb->error)
                break;
        }
        if (pkt->stream_index == audio_stream_) {
            audio_packets_.Put(pkt);
        } else if (pkt->stream_index == video_stream_) {
//            video_packets_.Put(pkt);
        } else {
            av_packet_unref(pkt);
        }
    }


}

void EasyPlayer::decodeAudio() {
    AVPacket pkt;
    AVFrame *frame = av_frame_alloc();
    while (true) {
        audio_packets_.Get(&pkt);
        int ret;
        ret = avcodec_send_packet(audio_codec_ctx_, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            ELOG("avcodec_send_packet error|code:%d", ret);
            break;
        }
        ret = avcodec_receive_frame(audio_codec_ctx_, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            ELOG("avcodec_receive_frame error|code:%d", ret);
            break;
        }
        audio_frames_.Put(frame);
    }
}

void EasyPlayer::decodeVideo(){
    AVPacket pkt;
    AVFrame *frame = av_frame_alloc();
    while (true) {
        video_packets_.Get(&pkt);
        int ret;
        ret = avcodec_send_packet(video_codec_ctx_, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            ELOG("avcodec_send_packet error|code:%d", ret);
            break;
        }
        ret = avcodec_receive_frame(video_codec_ctx_, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            ELOG("avcodec_receive_frame error|code:%d", ret);
            break;
        }
        video_frames_.Put(frame);
    }
}

void EasyPlayer::openStream(int index) {
    AVCodecContext *avctx;
    AVCodec *codec;
    int ret = 0;
    if (index < 0 || index >= ic_->nb_streams)
        return;
    avctx = avcodec_alloc_context3(NULL);
    if (!avctx) {
        ELOG("can not alloc codec ctx");
        return;
    }
    ret = avcodec_parameters_to_context(avctx, ic_->streams[index]->codecpar);
    if (ret < 0) {
        avcodec_free_context(&avctx);
        ELOG("avcodec_parameters_to_context error %d", ret);
        return;
    }
    av_codec_set_pkt_timebase(avctx, ic_->streams[index]->time_base);
    codec = avcodec_find_decoder(avctx->codec_id);
    avctx->codec_id = codec->id;
    ic_->streams[index]->discard = AVDISCARD_DEFAULT;
    ret = avcodec_open2(avctx, codec, NULL);
    if (ret < 0) {
        ELOG("Fail to open codec on stream:%d|code:%d", index, ret);
        avcodec_free_context(&avctx);
        return;
    }
    switch (avctx->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            swr_ctx_ = swr_alloc();
            swr_ctx_ = swr_alloc_set_opts(NULL,
                                         avctx->channel_layout, AV_SAMPLE_FMT_S16, avctx->sample_rate,
                                         avctx->channel_layout, avctx->sample_fmt, avctx->sample_rate,
                                         0, NULL);
            if (!swr_ctx_ || swr_init(swr_ctx_) < 0) {
                ELOG("Cannot create sample rate converter for conversion channels!");
                swr_free(&swr_ctx_);
                return;
            }
            audio_st_ = ic_->streams[index];
            audio_codec_ctx_ = avctx;
            audio_buffer_ = (uint8_t *) malloc(sizeof(uint8_t)*AUDIO_BUFFER_SIZE);
            initAudioPlayer(audio_codec_ctx_->sample_rate, audio_codec_ctx_->channels, EasyAudioPlayerCallback);
            audio_decode_thread_.reset(new std::thread(&EasyPlayer::decodeAudio, this));
            audio_render_thread_.reset(new std::thread(startAudioPlay, this));
            ILOG("start play audio");
            break;
        case AVMEDIA_TYPE_VIDEO:
            video_st_ = ic_->streams[index];
            img_convert_ctx_ = sws_getContext(avctx->width, avctx->height, avctx->pix_fmt,
                                             avctx->width, avctx->height, AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
            video_codec_ctx_ = avctx;
            video_decode_thread_.reset(new std::thread(&EasyPlayer::decodeVideo, this));
            break;
        default:
            break;
    }
}
