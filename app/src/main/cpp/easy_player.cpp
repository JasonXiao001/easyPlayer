//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/1/19.
//

#include "easy_player.h"
#include "log_util.h"
#include "common.h"


EasyPlayer::EasyPlayer() : state_(State::Idle), ic_(nullptr) {
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
    int err;
    err = avformat_open_input(&ic_, path.c_str(), NULL, NULL);
    if (err) {
        ELOG("avformat_open_input failed|ret:%d", err);
        return ERROR_IO;
    }
    state_ = State ::Initialized;
    return SUCCESS;
}

int EasyPlayer::PrepareAsync() {
    if (state_ != State::Initialized || state_ != State::Stoped) {
        ELOG("illegal state|current:%d", state_);
        return ERROR_ILLEGAL_STATE;
    }

    return SUCCESS;
}
