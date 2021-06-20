#pragma once

#include <media/NdkImageReader.h>

#include <opencv2/core.hpp>

#include "gl_program.h"
#include "logger.h"
#include "view.h"

class Renderer : public Logger {
   public:
    Renderer();
    void init_render();
    void updateSize_render(int width, int heigth);
    void drawFrame_render();
    void showView(ViewPtr view);
    int startLoop(std::vector<ViewPtr> views, double fps);
    void stopLoop(int loop_id);
    void deinit();

   private:
    void deinit_render();
    void updateImage_render();
    bool initTexture_render();
    void deinitTexture_render();
    void drawImage_render();
    void checkGlError_render();
    void stopLoop_();

    std::mutex mu_;
    cv::Mat image_rgb_ext_;
    bool image_updated_ext_ = false;

    cv::Mat image_rgb_;
    int image_width_ = 0;
    int image_height_ = 0;
    bool image_updated_ = false;

    int win_width_ = 0;
    int win_height_ = 0;

    GLProgram video_program_;
    std::atomic<bool> initialized_ = false;

    float vertices_[5 * 4]{
        // x, y, z, u, v
        -1, -1, 0, 0, 0,  // bottom left
        -1, 1,  0, 0, 1,  // top left
        1,  1,  0, 1, 1,  // top right
        1,  -1, 0, 1, 0   // bottom right
    };
    const unsigned bottom_left_ind_ = 3 + 5 * 0;
    const unsigned top_left_ind_ = 3 + 5 * 1;
    const unsigned top_right_ind_ = 3 + 5 * 2;
    const unsigned bottom_right_ind_ = 3 + 5 * 3;
    const unsigned u_ind_ = 0, v_ind_ = 1;

    unsigned indices_[6]{0, 1, 3, 1, 2, 3};

    bool texture_initialized_ = false;
    unsigned texture_id_ = 0;
    unsigned video_vertex_array_ = 0;
    unsigned video_buffers_[2]{0, 0};

    std::vector<ViewPtr> views_;
    double fps_ = 30.0;
    int i_view_ = 0;
    int cur_loop_id_ = 0;
    bool loop_is_active_ = false;
    double last_view_time_ = 0;
};

using RendererPtr = std::shared_ptr<Renderer>;

