#pragma once

#include <string>

class Logger {
   public:
    Logger(const char *tag = nullptr) noexcept;
    void V(const char *fmt, ...);
    void D(const char *fmt, ...);
    void I(const char *fmt, ...);
    void W(const char *fmt, ...);
    void E(const char *fmt, ...);

   protected:
    void setTag(std::string &&tag);

   private:
    std::string tag_;
};
