//
// Created by jasonxiao on 2018/1/24.
//

#ifndef EASYPLAYER_LOGUTIL_H
#define EASYPLAYER_LOGUTIL_H

#include <android/log.h>

#define LOG_LEVEL_V 1
#define LOG_LEVEL_D 2
#define LOG_LEVEL_I 3
#define LOG_LEVEL_W 4
#define LOG_LEVEL_E 5
#define LOG_LEVEL_N 6

#ifdef LOG_TAG
#define LOG_TAG __FILE__
#endif


class LogUtil {
public:
    static LogUtil& GetInstance()
    {
        static LogUtil instance;
        return instance;
    }
    void Log(int lvl, const char *tag, const char *fmt, ...);
private:
    LogUtil();
    ~LogUtil();
    LogUtil(const LogUtil&);
    LogUtil& operator=(const LogUtil&);
};


#endif //EASYPLAYER_LOGUTIL_H
