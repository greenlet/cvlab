#include "logger.h"

#include <cstdarg>
#include <typeinfo>
#include <android/log.h>


#define LOG_FUN(__fn_name, __prio) void Logger::__fn_name(const char *fmt, ...) { \
    va_list args; \
    va_start(args, fmt); \
    __android_log_vprint(ANDROID_LOG_##__prio, tag_, fmt, args); \
    va_end(args); \
}


Logger::Logger(const char *tag) noexcept : tag_(tag) {
    if (tag_ == nullptr) {
        tag_ = typeid(*this).name();
    }
}

LOG_FUN(V, VERBOSE)
LOG_FUN(D, DEBUG)
LOG_FUN(I, INFO)
LOG_FUN(W, WARN)
LOG_FUN(E, ERROR)

