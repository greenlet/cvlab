#pragma once

class Logger {
public:
    Logger(const char *tag=nullptr);
    void V(const char *fmt, ...);
    void D(const char *fmt, ...);
    void I(const char *fmt, ...);
    void W(const char *fmt, ...);
    void E(const char *fmt, ...);
private:
    const char *tag_;
};

