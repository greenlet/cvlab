#pragma once

#import "logger.h"

class GLProgram : public Logger {
   public:
    GLProgram(const char *name = nullptr);

    void init(const char *vertex_shader_src, const char *fragment_shader_src);
    void use();
    void deinit();
    void setBool(const char *name, bool value);
    void setInt(const char *name, int value);
    void setFloat(const char *name, float value);

    const char *name() { return name_; }
    const unsigned id() { return program_id_; }

    GLProgram(const GLProgram &) = delete;
    void operator=(const GLProgram &) = delete;

    ~GLProgram();

    void checkValidity();
    bool valid();

   private:
    void checkShaderStatus(unsigned shader_id);
    void checkProgramStatus();

    const char *name_;
    unsigned program_id_ = 0;
    static const unsigned status_buf_len_ = 2000;
    char status_buf_[status_buf_len_];
    int status_;
    bool initialized_ = false;
};
