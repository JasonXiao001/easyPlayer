//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/1/19.
//

#ifndef EASYPLAYER_EASY_PLAYER_H
#define EASYPLAYER_EASY_PLAYER_H

#include <thread>
#include <string>
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
#include "libavcodec/avfft.h"
};

enum class State {
    Idle,
    Initialized,
    Preparing,
    Prepared,
    Started,
    Paused,
    Stoped,
    Completed,
    End,
    Error,
};


class EasyPlayer {
public:
    EasyPlayer();
    EasyPlayer(const EasyPlayer&) = delete;
    ~EasyPlayer();
    int SetDataSource(const std::string &path);
    int PrepareAsync();

private:
    State state_;
    AVFormatContext *ic_;
};


#endif //EASYPLAYER_EASY_PLAYER_H
