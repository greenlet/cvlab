#pragma once

#include <camera/NdkCameraManager.h>
#include <media/NdkImageReader.h>

#include <opencv2/core.hpp>

#include "common.h"
#include "logger.h"
#include "view.h"

struct CameraSize {
    int32_t width;
    int32_t height;

    std::string toString() const;
};

struct CameraProps {
    std::string id;
    int64_t min_exposure = 0;
    int64_t max_exposure = 0;
    int32_t min_sensitivity = 0;
    int32_t max_sensitivity = 0;

    std::vector<CameraSize> jpeg_sizes;
    std::vector<CameraSize> yuv_420_888_sizes;

    int32_t orientation = 0;

    static std::string sizesToString(const std::vector<CameraSize> &sizes);
    std::string toString(int tab_size = 4) const;
};

using CameraCallback = std::function<void(ViewPtr)>;

class CameraAndroid : Logger {
   public:
    CameraAndroid();
    void setCallback(CameraCallback callback);
    void start();
    void stop();

   private:
    static void cameraDisconnectedCallback(void *context, ACameraDevice *device);
    static void cameraErrorCallback(void *context, ACameraDevice *device, int error);
    static void cameraImageCallback(void *context, AImageReader *image_reader);
    static void captureSessionClosedCallback(void *context, ACameraCaptureSession *session);
    static void captureSessionReadyCallback(void *context, ACameraCaptureSession *session);
    static void captureSessionActiveCallback(void *context, ACameraCaptureSession *session);

    void onDisconnected(ACameraDevice *camera_device);
    void onError(ACameraDevice *device, int error);
    void onNewImage(AImageReader *image_reader);
    void onCaptureSessionClosed(ACameraCaptureSession *session);
    void onCaptureSessionReady(ACameraCaptureSession *session);
    void onCaptureSessionActive(ACameraCaptureSession *session);

    CameraSize chooseDimensions(CameraSize preferrable, const std::vector<CameraSize> &sizes);
    void clear();

    std::mutex cam_mutex_;
    bool started_ = false;

    CameraCallback callback_;

    CameraProps camera_props_ = {};
    CameraSize camera_size_ = {};

    camera_status_t status_ = ACAMERA_OK;
    ACameraManager *camera_manager_ = nullptr;
    ACameraDevice *camera_device_ = nullptr;
    AImageReader *image_reader_ = nullptr;
    ANativeWindow *image_window_ = nullptr;
    ACameraOutputTarget *output_target_ = nullptr;
    ACaptureRequest *capture_request_ = nullptr;
    ACaptureSessionOutputContainer *capture_outputs_ = nullptr;
    ACaptureSessionOutput *capture_output_ = nullptr;
    ACameraCaptureSession *capture_session_ = nullptr;
    bool capturing_started_ = false;

    uint8_t *image_buffer_ = nullptr;
    int image_buffer_len_ = 0;
    int view_id_ = 0;
};
