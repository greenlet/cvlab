#include "renderer.h"
//#define GL_GLEXT_PROTOTYPES
//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include <utility>
#include "common.h"

Renderer::Renderer() : Logger("Renderer"), video_program_("VideoPreview") {}

void Renderer::init_render() {
    if (initialized_) {
        return;
    }
    I("Initializing");

    video_program_.init(R"(# version 300 es
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;

        out vec2 texCoord;

        void main()
        {
            gl_Position = vec4(aPos, 1.0);
            texCoord = aTexCoord;
        }
  )",
                        R"(# version 300 es
        precision mediump float;
        out vec4 FragColor;

        in vec3 ourColor;
        in vec2 texCoord;

        uniform sampler2D texSampler;

        void main()
        {
            FragColor = texture(texSampler, texCoord);
        }
  )");

    glViewport(0, 0, win_width_, win_height_);

    initialized_ = true;
}

void Renderer::updateSize_render(int width, int heigth) {
    deinit_render();
    win_width_ = width;
    win_height_ = heigth;
    init_render();
}

bool Renderer::initTexture_render() {
    if (!texture_initialized_) {
        video_program_.use();

        glGenTextures(1, &texture_id_);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenVertexArrays(1, &video_vertex_array_);
        glGenBuffers(2, video_buffers_);

        float image_width = float(image_width_), image_height = float(image_height_);
        float scale = std::min(float(win_width_) / image_width, float(win_height_) / image_height);
        image_width *= scale;
        image_height *= scale;
        float h_offset = (image_width - float(win_width_)) / 2;
        float v_offset = (image_height - float(win_height_)) / 2;
        vertices_[bottom_left_ind_ + u_ind_] = h_offset / image_width;
        vertices_[bottom_left_ind_ + v_ind_] = v_offset / image_height;
        vertices_[top_left_ind_ + u_ind_] = h_offset / image_width;
        vertices_[top_left_ind_ + v_ind_] = (image_height - v_offset) / image_height;
        vertices_[top_right_ind_ + u_ind_] = (image_width - h_offset) / image_width;
        vertices_[top_right_ind_ + v_ind_] = (image_height - v_offset) / image_height;
        vertices_[bottom_right_ind_ + u_ind_] = (image_width - h_offset) / image_width;
        vertices_[bottom_right_ind_ + v_ind_] = v_offset / image_height;

        glBindVertexArray(video_vertex_array_);
        glBindBuffer(GL_ARRAY_BUFFER, video_buffers_[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), vertices_, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, video_buffers_[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_), indices_, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                              (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        video_program_.setInt("texSampler", 0);

        texture_initialized_ = true;
        return true;
    }
    return false;
}

void Renderer::deinitTexture_render() {
    if (texture_initialized_) {
        glDeleteTextures(1, &texture_id_);
        glDeleteVertexArrays(1, &video_vertex_array_);
        glDeleteBuffers(2, video_buffers_);
        texture_initialized_ = false;
        image_updated_ = false;
        image_rgb_ = cv::Mat();
        image_width_ = 0;
        image_height_ = 0;
    }
}

void Renderer::drawImage_render() {
    if (win_width_ == 0 || win_height_ == 0 || image_rgb_.empty()) {
        return;
    }

    if (image_rgb_.size[1] != image_width_ || image_rgb_.size[0] != image_height_) {
        deinitTexture_render();
        image_width_ = image_rgb_.size[1];
        image_height_ = image_rgb_.size[0];
    }

    video_program_.use();

    bool first_time = initTexture_render();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id_);

    if (first_time) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width_, image_height_, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, image_rgb_.data);
    } else if (image_updated_) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_width_, image_height_, GL_RGB,
                        GL_UNSIGNED_BYTE, image_rgb_.data);
        image_updated_ = false;
    };

    checkGlError_render();

    glBindVertexArray(video_vertex_array_);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

char const *glErrorToString(GLenum const err) noexcept {
    switch (err) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";

        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";

        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";

        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";

        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";

        default:
            return nullptr;
    }
}

void Renderer::checkGlError_render() {
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char *err_str = glErrorToString(err);
        if (err_str) {
            E("GL error: %s", err_str);
        } else {
            E("GL error: %#06x", err);
        }
    }
}

void Renderer::drawFrame_render() {
    if (!initialized_) {
        return;
    }
    updateImage_render();
    if (!initialized_) {
        return;
    }
    //  D("drawFrame. image_updated: %d", image_updated_);
    drawImage_render();
}

void Renderer::updateImage_render() {
    G_LOCK(mu_);
    if (image_updated_ext_) {
        image_rgb_ = image_rgb_ext_;
        image_updated_ext_ = false;
        image_updated_ = true;
    } else if (loop_is_active_) {
        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> now_ms = now.time_since_epoch();
        double elapsed = now_ms.count() - last_view_time_;
        if (last_view_time_ == 0 || elapsed >= 1000 / fps_) {
            image_rgb_ = views_[i_view_]->img();
            image_updated_ = true;
            i_view_ = (i_view_ + 1) % views_.size();
            last_view_time_ = now_ms.count();
        }
    }
}

void Renderer::deinit_render() {
    if (initialized_) {
        video_program_.deinit();
        deinitTexture_render();
        initialized_ = false;
    }
}

void Renderer::deinit() {
    G_LOCK(mu_);
    initialized_ = false;
}

void Renderer::showView(ViewPtr view) {
    G_LOCK(mu_);
    stopLoop_();
    image_rgb_ext_ = view->img();
    image_updated_ext_ = true;
}

int Renderer::startLoop(std::vector<ViewPtr> views, double fps) {
    G_LOCK(mu_);
    if (views.empty()) {
        W("Calling startLoop with empty views container");
        return -1;
    }
    stopLoop_();
    cur_loop_id_++;
    loop_is_active_ = true;
    views_ = views;
    i_view_ = 0;
    fps_ = fps;
    last_view_time_ = 0;
    return cur_loop_id_;
}

void Renderer::stopLoop(int loop_id) {
    G_LOCK(mu_);
    if (cur_loop_id_ == loop_id) {
        stopLoop_();
    }
}

void Renderer::stopLoop_() {
    if (loop_is_active_) {
        loop_is_active_ = false;
        views_.clear();
    }
}
