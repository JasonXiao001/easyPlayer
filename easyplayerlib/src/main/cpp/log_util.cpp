//
// Created by jasonxiao on 2018/1/24.
//

#include "log_util.h"
#include <android/log.h>
#include <string>

int LogUtil::log_lvl_ = LOG_LEVEL_V;

void LogUtil::Log(int lvl, const char *file, const unsigned line, const char *fmt, ...) {
    if (lvl < log_lvl_) return;
    char tag[256];
    sprintf(tag, "%s:%u", file, line);
    va_list vl;
    va_start(vl, fmt);
    LogVl(lvl, tag, fmt, vl);
    va_end(vl);
}

void LogUtil::LogVl(int lvl, const char *tag, const char *fmt, va_list vl) {
    if (lvl < log_lvl_) return;
    auto android_log_lvl = ANDROID_LOG_UNKNOWN;
    switch(lvl)
    {
        case LOG_LEVEL_V:
        {
            android_log_lvl = ANDROID_LOG_VERBOSE;
            break;
        }
        case LOG_LEVEL_D:
        {
            android_log_lvl = ANDROID_LOG_DEBUG;
            break;
        }
        case LOG_LEVEL_I:
        {
            android_log_lvl = ANDROID_LOG_INFO;
            break;
        }
        case LOG_LEVEL_W:
        {
            android_log_lvl = ANDROID_LOG_WARN;
            break;
        }
        case LOG_LEVEL_E:
        {
            android_log_lvl = ANDROID_LOG_ERROR;
            break;
        }
        case LOG_LEVEL_N:
        {
            android_log_lvl = ANDROID_LOG_SILENT;
            break;
        }
        default:
            android_log_lvl = ANDROID_LOG_DEFAULT;
    }
    __android_log_vprint(android_log_lvl, tag, fmt, vl);
}








