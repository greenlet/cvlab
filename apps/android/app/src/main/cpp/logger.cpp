#include "logger.h"

#include <stdarg.h>
#include <typeinfo>
#include <android/log.h>


#define LOG_FUN(__fn_name, __prio) void Logger::__fn_name(const char *fmt, ...) { \
    va_list args; \
    va_start(args, fmt); \
    __android_log_vprint(ANDROID_LOG_##__prio, tag_, fmt, args); \
    va_end(args); \
}


Logger::Logger(const char *tag) : tag_(tag) {
    if (tag_ == nullptr) {
        tag_ = typeid(*this).name();
    }
}

LOG_FUN(lv, VERBOSE)
LOG_FUN(ld, DEBUG)
LOG_FUN(li, INFO)
LOG_FUN(lw, WARN)
LOG_FUN(le, ERROR)

