#include "camera_android.h"

#include <camera/NdkCameraManager.h>
#include <media/NdkImage.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <utility>

static const char *TAG = "CameraAndroid";
static Logger LOG(TAG);

std::string getBackFacingCameraId(ACameraManager *camera_manager) {
    ACameraIdList *camera_ids = nullptr;
    ACameraManager_getCameraIdList(camera_manager, &camera_ids);

    LOG.D("Cameras:");
    const char *back_camera_id = nullptr;
    for (int i = 0; i < camera_ids->numCameras; i++) {
        const char *camera_id = camera_ids->cameraIds[i];
        ACameraMetadata *camera_metadata;
        ACameraManager_getCameraCharacteristics(camera_manager, camera_id, &camera_metadata);
        LOG.D("Camera %s", camera_id);
        int32_t tags_n;
        const uint32_t *tags = nullptr;
        ACameraMetadata_getAllTags(camera_metadata, &tags_n, &tags);

        for (int tag_ind = 0; tag_ind < tags_n; tag_ind++) {
            if (tags[tag_ind] == ACAMERA_LENS_FACING) {
                ACameraMetadata_const_entry lens_info = {0};
                camera_status_t status =
                    ACameraMetadata_getConstEntry(camera_metadata, tags[tag_ind], &lens_info);
                if (status != ACAMERA_OK) {
                    continue;
                }
                const uint8_t facing =
                    static_cast<acamera_metadata_enum_android_lens_facing_t>(lens_info.data.u8[0]);
                if (facing == ACAMERA_LENS_FACING_BACK) {
                    back_camera_id = camera_id;
                    LOG.D("Camera facing back!");
                }
            }
        }

        ACameraMetadata_free(camera_metadata);
    }

    ACameraManager_deleteCameraIdList(camera_ids);

    return back_camera_id;
}

CameraProps getCameraProps(ACameraManager *camera_manager, const char *camera_id) {
    CameraProps camera_props;
    camera_props.id = camera_id;
    camera_status_t status;

    // Exposure range
    ACameraMetadata *camera_metadata;
    ACameraManager_getCameraCharacteristics(camera_manager, camera_id, &camera_metadata);

    ACameraMetadata_const_entry entry = {};
    status = ACameraMetadata_getConstEntry(camera_metadata, ACAMERA_SENSOR_INFO_EXPOSURE_TIME_RANGE,
                                           &entry);
    if (status != ACAMERA_OK) {
        LOG.W("Exposure time is not supported for this camera");
    }

    camera_props.min_exposure = entry.data.i64[0];
    camera_props.max_exposure = entry.data.i64[1];
    ////////////////////////////////////////////////////////////////

    // Sensitivity
    ACameraMetadata_getConstEntry(camera_metadata, ACAMERA_SENSOR_INFO_SENSITIVITY_RANGE, &entry);

    camera_props.min_sensitivity = entry.data.i32[0];
    camera_props.max_sensitivity = entry.data.i32[1];
    ////////////////////////////////////////////////////////////////

    // Image format available sizes
    ACameraMetadata_getConstEntry(camera_metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
                                  &entry);

    for (int i = 0; i < entry.count; i += 4) {
        // We are only interested in output streams, so skip input stream
        int32_t input = entry.data.i32[i + 3];
        if (input) continue;

        int32_t format = entry.data.i32[i + 0];
        int32_t max_width = entry.data.i32[i + 1];
        int32_t max_height = entry.data.i32[i + 2];

        if (format == AIMAGE_FORMAT_JPEG) {
            camera_props.jpeg_sizes.push_back(CameraSize{max_width, max_height});
        } else if (format == AIMAGE_FORMAT_YUV_420_888) {
            camera_props.yuv_420_888_sizes.push_back(CameraSize{max_width, max_height});
        }
    }

    // Orientation
    ACameraMetadata_getConstEntry(camera_metadata, ACAMERA_SENSOR_ORIENTATION, &entry);

    camera_props.orientation = entry.data.i32[0];

    ACameraMetadata_free(camera_metadata);

    return camera_props;
}

std::string CameraSize::toString() const {
    std::ostringstream os;
    os << width << " x " << height;
    return os.str();
}

std::string CameraProps::sizesToString(const std::vector<CameraSize> &sizes) {
    if (sizes.empty()) {
        return "<empty>";
    }
    std::ostringstream os;
    for (int i = 0; i < sizes.size(); i++) {
        const auto &sz = sizes[i];
        os << sz.width << " x " << sz.height;
        if (i < sizes.size() - 1) {
            os << ", ";
        }
    }
    return os.str();
}

std::string CameraProps::toString(int tab_size) const {
    std::ostringstream os;
    std::string tab(tab_size, ' ');
    os << "Camera: " << id << ". Properties:" << std::endl;
    os << "Exposure: min = " << min_exposure << ", max = " << max_exposure << std::endl;
    os << "Sensitivity: min = " << min_sensitivity << ", max = " << max_sensitivity << std::endl;
    os << "JPEG sizes:\n" << tab << sizesToString(jpeg_sizes) << std::endl;
    os << "YUV_420_888 sizes:\n" << tab << sizesToString(yuv_420_888_sizes) << std::endl;
    os << "Orientation: deg = " << orientation << std::endl;
    return os.str();
}

CameraSize CameraAndroid::chooseDimensions(CameraSize preferrable,
                                           const std::vector<CameraSize> &sizes) {
    CameraSize res = {};
    int32_t min_diff = 1e6;
    for (const auto &size : sizes) {
        int32_t diff = abs(preferrable.width - size.width) + abs(preferrable.height - size.height);
        if (res.width == 0 || diff < min_diff) {
            res = size;
            min_diff = diff;
        }
    }
    return res;
}

CameraAndroid::CameraAndroid() : Logger(TAG) {}

void CameraAndroid::setCallback(CameraCallback callback) { callback_ = std::move(callback); }

void CameraAndroid::cameraImageCallback(void *context, AImageReader *reader) {
    static_cast<CameraAndroid *>(context)->onNewImage(reader);
}

void CameraAndroid::cameraDisconnectedCallback(void *context, ACameraDevice *device) {
    static_cast<CameraAndroid *>(context)->onDisconnected(device);
}

void CameraAndroid::cameraErrorCallback(void *context, ACameraDevice *device, int error) {
    static_cast<CameraAndroid *>(context)->onError(device, error);
}

void CameraAndroid::captureSessionClosedCallback(void *context, ACameraCaptureSession *session) {
    static_cast<CameraAndroid *>(context)->onCaptureSessionClosed(session);
}

void CameraAndroid::captureSessionReadyCallback(void *context, ACameraCaptureSession *session) {
    static_cast<CameraAndroid *>(context)->onCaptureSessionReady(session);
}

void CameraAndroid::captureSessionActiveCallback(void *context, ACameraCaptureSession *session) {
    static_cast<CameraAndroid *>(context)->onCaptureSessionActive(session);
}

void CameraAndroid::onDisconnected(ACameraDevice *camera_device) { LOG.I("Camera disconnected"); }

void CameraAndroid::onError(ACameraDevice *device, int error) { LOG.E("Camera error: %d", error); }

void CameraAndroid::onNewImage(AImageReader *image_reader) {
    std::unique_lock<std::mutex> lock(cam_mutex_);
    if (!started_) {
        clear();
        return;
    }
    AImage *image_src;
    AImageReader_acquireNextImage(image_reader, &image_src);

    int32_t width, height, format;
    AImage_getWidth(image_src, &width);
    AImage_getHeight(image_src, &height);
    AImage_getFormat(image_src, &format);

    uint8_t *y_pixel, *u_pixel, *v_pixel;
    int y_len, u_len, v_len;
    AImage_getPlaneData(image_src, 0, &y_pixel, &y_len);
    AImage_getPlaneData(image_src, 1, &u_pixel, &u_len);
    AImage_getPlaneData(image_src, 2, &v_pixel, &v_len);
    //  int32_t n_planes, y_stride, u_stride, v_stride;
    //  AImage_getNumberOfPlanes(image_src, &n_planes);
    //  AImage_getPlanePixelStride(image_src, 0, &y_stride);
    //  AImage_getPlanePixelStride(image_src, 0, &u_stride);
    //  AImage_getPlanePixelStride(image_src, 0, &v_stride);
    //  D("Planes: %d. y_stride: %d, u_stride: %d, v_stride: %d", n_planes, y_stride, u_stride,
    //  v_stride);

    cv::Mat image_rgb;

    int buf_len = y_len + u_len + v_len;
    bool rotate = (camera_props_.orientation / 90) % 2 == 1;
    if (image_buffer_len_ != buf_len) {
        if (image_buffer_len_ > 0) {
            delete[] image_buffer_;
        }
        image_buffer_ = new uint8_t[buf_len];
        image_buffer_len_ = buf_len;
    }

    memcpy(image_buffer_, y_pixel, y_len);
    memcpy(image_buffer_ + y_len, u_pixel, u_len);
    memcpy(image_buffer_ + y_len + u_len, v_pixel, v_len);

    AImage_delete(image_src);
    lock.unlock();

    cv::Mat image_yuv;
    if (rotate) {
        image_yuv = cv::Mat(height + height / 2, width, CV_8UC1, image_buffer_);
    } else {
        image_yuv = cv::Mat(height, width + width / 2, CV_8UC1, image_buffer_);
    }
    cv::cvtColor(image_yuv, image_rgb, cv::COLOR_YUV2RGB_NV21, 3);

    // TODO: Different for emulator
    cv::flip(image_rgb, image_rgb, 1);

    if (rotate) {
        cv::rotate(image_rgb, image_rgb, cv::ROTATE_90_CLOCKWISE);
    }
    cv::cvtColor(image_rgb, image_rgb, cv::COLOR_BGR2RGB);

    //  D("onNewImage: format = %d. y = %d, u = %d, v = %d. y + u + v = %d. width = %d, height =
    //  %d, 1.5 * width * height = %d",
    //    format, y_len, u_len, v_len, buf_len, width, height, (width + width / 2) * height);
    //  D("y: %p - %p. u: %p - %p. v: %p - %p", y_pixel, y_pixel + y_len - 1, u_pixel, u_pixel +
    //  u_len - 1, v_pixel, v_pixel + v_len - 1);

    callback_(image_rgb);
}

void CameraAndroid::onCaptureSessionClosed(ACameraCaptureSession *session) {
    D("onCaptureSessionClosed");
}

void CameraAndroid::onCaptureSessionReady(ACameraCaptureSession *session) {
    D("onCaptureSessionReady");
}

void CameraAndroid::onCaptureSessionActive(ACameraCaptureSession *session) {
    D("onCaptureSessionActive");
}

void CameraAndroid::start() {
    std::lock_guard<std::mutex> lock(cam_mutex_);
    if (started_) {
        return;
    }
    started_ = true;

    camera_manager_ = ACameraManager_create();
    std::string camera_id = getBackFacingCameraId(camera_manager_);
    camera_props_ = getCameraProps(camera_manager_, camera_id.c_str());
    LOG.I(camera_props_.toString().c_str());

    ACameraDevice_stateCallbacks camera_device_callbacks{
        this,
        cameraDisconnectedCallback,
        cameraErrorCallback,
    };
    status_ = ACameraManager_openCamera(camera_manager_, camera_props_.id.c_str(),
                                        &camera_device_callbacks, &camera_device_);
    if (status_ != ACAMERA_OK) {
        clear();
        return;
    }

    camera_size_ = chooseDimensions(CameraSize{1920, 1080}, camera_props_.yuv_420_888_sizes);
    LOG.I("Size chosen: %s", camera_size_.toString().c_str());

    AImageReader_new(camera_size_.width, camera_size_.height, AIMAGE_FORMAT_YUV_420_888, 4,
                     &image_reader_);
    AImageReader_ImageListener image_callbacks{.context = this,
                                               .onImageAvailable = cameraImageCallback};
    AImageReader_setImageListener(image_reader_, &image_callbacks);
    AImageReader_getWindow(image_reader_, &image_window_);
    ANativeWindow_acquire(image_window_);
    ACameraOutputTarget_create(image_window_, &output_target_);

    ACameraDevice_createCaptureRequest(camera_device_, TEMPLATE_PREVIEW, &capture_request_);
    ACaptureRequest_addTarget(capture_request_, output_target_);
    ACaptureSessionOutput_create(image_window_, &capture_output_);
    ACaptureSessionOutputContainer_create(&capture_outputs_);
    ACaptureSessionOutputContainer_add(capture_outputs_, capture_output_);

    ACameraCaptureSession_stateCallbacks capture_session_state_callbacks{
        .context = this,
        .onClosed = captureSessionClosedCallback,
        .onReady = captureSessionReadyCallback,
        .onActive = captureSessionActiveCallback,
    };
    ACameraDevice_createCaptureSession(camera_device_, capture_outputs_,
                                       &capture_session_state_callbacks, &capture_session_);

    ACameraCaptureSession_captureCallbacks capture_session_capture_callbacks{
        .context = nullptr,
        .onCaptureStarted = nullptr,
        .onCaptureProgressed = nullptr,
        .onCaptureCompleted = nullptr,
        .onCaptureFailed = nullptr,
        .onCaptureSequenceCompleted = nullptr,
        .onCaptureSequenceAborted = nullptr,
        .onCaptureBufferLost = nullptr,
    };
    ACameraCaptureSession_setRepeatingRequest(capture_session_, &capture_session_capture_callbacks,
                                              1, &capture_request_, nullptr);
    capturing_started_ = true;
}

void CameraAndroid::clear() {
    if (image_buffer_ != nullptr) {
        delete[] image_buffer_;
        image_buffer_ = nullptr;
        image_buffer_len_ = 0;
    }

    if (capture_session_ != nullptr) {
        if (capturing_started_) {
            ACameraCaptureSession_abortCaptures(capture_session_);
            capturing_started_ = false;
        }
        ACameraCaptureSession_close(capture_session_);
        capture_session_ = nullptr;
    }
    if (capture_outputs_ != nullptr) {
        ACaptureSessionOutputContainer_free(capture_outputs_);
        capture_outputs_ = nullptr;
    }
    if (capture_output_ != nullptr) {
        ACaptureSessionOutput_free(capture_output_);
        capture_output_ = nullptr;
    }
    if (capture_request_ != nullptr) {
        ACaptureRequest_free(capture_request_);
        capture_request_ = nullptr;
    }
    if (output_target_ != nullptr) {
        ACameraOutputTarget_free(output_target_);
        output_target_ = nullptr;
    }
    if (image_reader_ != nullptr) {
        AImageReader_delete(image_reader_);
        image_reader_ = nullptr;
        image_window_ = nullptr;
    }
    if (camera_device_ != nullptr) {
        ACameraDevice_close(camera_device_);
        camera_device_ = nullptr;
    }
    if (camera_manager_ != nullptr) {
        ACameraManager_delete(camera_manager_);
        camera_manager_ = nullptr;
    }
}

void CameraAndroid::stop() {
    std::lock_guard<std::mutex> lock(cam_mutex_);
    if (!started_) {
        return;
    }
    started_ = false;

    //  clear();
}
