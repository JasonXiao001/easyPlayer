//
// Created by jasonxiao{github.com/JasonXiao001} on 2018/1/23.
//

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
        }
        if (ic_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            st_index[AVMEDIA_TYPE_VIDEO] = i;
            av_log(NULL, AV_LOG_INFO, "start open video component at id %d.\n",st_index[AVMEDIA_TYPE_VIDEO]);
            video_stream = new Stream(i, ic_);
        }
    }
    if (video_stream < 0 && audio_stream < 0) {
        av_log(NULL, AV_LOG_FATAL, "Failed to open file '%s' or configure filtergraph\n", data_source_.c_str());
        release();
    }
}

void Player::read() {

}

void Player::release() {

}
