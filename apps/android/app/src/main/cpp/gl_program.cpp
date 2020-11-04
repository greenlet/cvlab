#include "gl_program.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

GLProgram::GLProgram(const char *name) : name_(name) {
    std::string logger_name = "GLProgram";
    if (name) {
        logger_name = logger_name + "_" + name;
    }
    setTag(std::move(logger_name));
}

void GLProgram::checkShaderStatus(unsigned shader_id) {
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status_);
    if (!status_) {
        glGetShaderInfoLog(shader_id, status_buf_len_, NULL, status_buf_);
        E("Shader compilation failed\n%s", status_buf_);
    }
}

void GLProgram::checkProgramStatus() {
    glGetProgramiv(program_id_, GL_LINK_STATUS, &status_);
    if (!status_) {
        glGetProgramInfoLog(program_id_, status_buf_len_, NULL, status_buf_);
        E("Program compilation failed\n%s", status_buf_);
    }
}

bool GLProgram::valid() {
    glValidateProgram(program_id_);
    glGetProgramiv(program_id_, GL_VALIDATE_STATUS, &status_);
    return status_;
}

void GLProgram::checkValidity() {
    if (!valid()) {
        E("Program invalid");
    } else {
        I("Program valid");
    }
}

void GLProgram::init(const char *vertex_shader_src, const char *fragment_shader_src) {
    if (initialized_) {
        return;
    }

    unsigned vertex_shader_id;
    vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_id, 1, &vertex_shader_src, NULL);
    glCompileShader(vertex_shader_id);
    checkShaderStatus(vertex_shader_id);

    unsigned fragment_shader_id;
    fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_id, 1, &fragment_shader_src, NULL);
    glCompileShader(fragment_shader_id);
    checkShaderStatus(fragment_shader_id);

    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vertex_shader_id);
    glAttachShader(program_id_, fragment_shader_id);
    glLinkProgram(program_id_);
    checkProgramStatus();

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    initialized_ = true;
}

void GLProgram::use() {
    if (!initialized_) {
        E("Trying to call use() on noninitialized "
          "program");
        return;
    }
    glUseProgram(program_id_);
}

void GLProgram::setBool(const char *name, bool value) {
    if (!initialized_) {
        E("Trying to call setBool() on "
          "noninitialized program");
        return;
    }
    glUniform1i(glGetUniformLocation(program_id_, name), (int)value);
}

void GLProgram::setInt(const char *name, int value) {
    if (!initialized_) {
        E("Trying to call setInt() on noninitialized "
          "program");
        return;
    }
    glUniform1i(glGetUniformLocation(program_id_, name), value);
}

void GLProgram::setFloat(const char *name, float value) {
    if (!initialized_) {
        E("Trying to call setFloat() on "
          "noninitialized program");
        return;
    }
    glUniform1f(glGetUniformLocation(program_id_, name), value);
}

void GLProgram::deinit() {
    if (initialized_) {
        glDeleteProgram(program_id_);
        initialized_ = false;
    }
}

GLProgram::~GLProgram() { deinit(); }
