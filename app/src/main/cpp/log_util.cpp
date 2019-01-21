//
// Created by jasonxiao{github.com/JasonXiao001} on 2019/1/20.
//

#include "log_util.h"
#include <string>
#include <sstream>
extern "C" {
#include "libavutil/log.h"
};

void ff_log_callback(void* ptr, int level, const char* fmt, va_list vl) {
    switch (level) {
        case AV_LOG_VERBOSE:
            __android_log_vprint(ANDROID_LOG_VERBOSE,  FF_LOG_TAG, fmt, vl);
            break;
        case AV_LOG_DEBUG:
            __android_log_vprint(ANDROID_LOG_DEBUG,  FF_LOG_TAG, fmt, vl);
            break;
        case AV_LOG_INFO:
            __android_log_vprint(ANDROID_LOG_INFO,  FF_LOG_TAG, fmt, vl);
            break;
        case AV_LOG_WARNING:
            __android_log_vprint(ANDROID_LOG_WARN,  FF_LOG_TAG, fmt, vl);
            break;
        case AV_LOG_ERROR:
        case AV_LOG_FATAL:
        case AV_LOG_PANIC:
            __android_log_vprint(ANDROID_LOG_FATAL,  FF_LOG_TAG, fmt, vl);
            break;
        case AV_LOG_QUIET:
            __android_log_vprint(ANDROID_LOG_SILENT,  FF_LOG_TAG, fmt, vl);
            break;
        default:
            __android_log_vprint(ANDROID_LOG_VERBOSE,  FF_LOG_TAG, fmt, vl);
            break;
    }
}

void WriteLog(LogLevel level, const char *tag, const char *file_name, const char *func_name, int line, const char *fmt, ...) {
    if (level >= MY_LOG_LEVEL) {
        std::string file_name_no_path(file_name);
        if (file_name_no_path.rfind("\\") != std::string::npos) {
            file_name_no_path = file_name_no_path.substr(file_name_no_path.rfind("\\") + 1);
        }else {
            file_name_no_path = file_name_no_path.substr(file_name_no_path.rfind("/") + 1);
        }
        char buf[1024];
        std::string log;
        va_list ap;
        va_start(ap, fmt);
        int write_n = vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        if (write_n >= 0 && write_n < static_cast<int>(sizeof(buf))) {
            log = std::string(buf);
        }else {
            log = "log too long";
        }
        std::ostringstream ss;
        ss << file_name_no_path << ":" << line << "|" << func_name << ": " << log;
        __android_log_write(level, tag, ss.str().c_str());
    }
}

