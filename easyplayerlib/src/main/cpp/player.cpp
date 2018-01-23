//
// Created by jasonxiao on 2018/1/23.
//

#include "player.h"




void Player::SetDataSource(const std::string &data_source) {
    data_source_ = data_source;

}

Player::Player() {
}

Player::~Player() {
}

void Player::Prepare() {
    av_register_all();
    avformat_network_init();
}

void Player::read() {
    int err, i, ret;
    av_log(NULL, AV_LOG_INFO, "start read thread.\n");
    AVPacket *pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
    if (pkt == NULL) {
        av_log(NULL, AV_LOG_FATAL, "Could not allocate avPacket.\n");
        return;
    }
    int64_t stream_start_time;
    int64_t pkt_ts;
    int pkt_in_play_range = 0;
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
        }
        if (ic_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            st_index[AVMEDIA_TYPE_VIDEO] = i;
        }
    }
    if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
        av_log(NULL, AV_LOG_INFO, "start open video component at id %d.\n",st_index[AVMEDIA_TYPE_VIDEO]);
        stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
    }
    if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
        av_log(NULL, AV_LOG_INFO, "start open audio component at id %d.\n",st_index[AVMEDIA_TYPE_AUDIO]);
        stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
    }
    if (video_stream < 0 && audio_stream < 0) {
        av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n", filename);
        release();
    }
}

void Player::release() {

}
