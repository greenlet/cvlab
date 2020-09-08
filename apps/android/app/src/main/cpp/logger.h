#pragma once

class Logger {
public:
    Logger(const char *tag=nullptr);
    void lv(const char *fmt, ...);
    void ld(const char *fmt, ...);
    void li(const char *fmt, ...);
    void lw(const char *fmt, ...);
    void le(const char *fmt, ...);
private:
    const char *tag_;
};

