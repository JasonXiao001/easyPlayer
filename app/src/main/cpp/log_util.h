//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/1/19.
//

#ifndef EASYPLAYER_LOG_H
#define EASYPLAYER_LOG_H

#include <android/log.h>

enum LogLevel {
    kVerbose	=	2,
    kDebug		=	3,
    kInfo		=	4,
    kWarn		=	5,
    kError		=	6,
    kOff 		= 	8,
};

#ifndef MY_LOG_LEVEL
#define MY_LOG_LEVEL kVerbose
#endif
#ifndef MY_LOG_TAG
#define MY_LOG_TAG "easy_player"
#endif
#ifndef FF_LOG_TAG
#define FF_LOG_TAG "ffmpeg"
#endif


void ff_log_callback(void* ptr, int level, const char* fmt, va_list vl);

void WriteLog(LogLevel level, const char *tag, const char *file_name, const char *func_name, int line, const char *fmt, ...);


#define MLOG(level, tag, ...) WriteLog(level, tag, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);

#define ELOG(...) MLOG(kError, MY_LOG_TAG, __VA_ARGS__)
#define WLOG(...) MLOG(kWarn, MY_LOG_TAG, __VA_ARGS__)
#define ILOG(...) MLOG(kInfo, MY_LOG_TAG, __VA_ARGS__)
#define DLOG(...) MLOG(kDebug, MY_LOG_TAG, __VA_ARGS__)
#define VLOG(...) MLOG(kVerbose, MY_LOG_TAG, __VA_ARGS__)




#endif //EASYPLAYER_LOG_H
