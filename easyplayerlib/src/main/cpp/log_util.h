//
// Created by jasonxiao on 2018/1/24.
//

#ifndef EASYPLAYER_LOGUTIL_H
#define EASYPLAYER_LOGUTIL_H


#include <stdarg.h>

#define LOG_LEVEL_V 1
#define LOG_LEVEL_D 2
#define LOG_LEVEL_I 3
#define LOG_LEVEL_W 4
#define LOG_LEVEL_E 5
#define LOG_LEVEL_N 6


#define VLOG(fmt,...) \
    LogUtil::Log(LOG_LEVEL_V, __FILE__, __LINE__, fmt, __VA_ARGS__);

#define DLOG(fmt,...) \
    LogUtil::Log(LOG_LEVEL_D, __FILE__, __LINE__, fmt, __VA_ARGS__);

#define ILOG(fmt,...) \
    LogUtil::Log(LOG_LEVEL_I, __FILE__, __LINE__, fmt, __VA_ARGS__);

#define WLOG(fmt,...) \
    LogUtil::Log(LOG_LEVEL_W, __FILE__, __LINE__, fmt, __VA_ARGS__);

#define ELOG(fmt,...) \
    LogUtil::Log(LOG_LEVEL_E, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

class LogUtil {
public:
    static int log_lvl_;
    static void Log(int lvl, const char *file, const unsigned line, const char *fmt, ...);
    static void LogVl(int lvl, const char *tag, const char *fmt, va_list vl);
};


#endif //EASYPLAYER_LOGUTIL_H
