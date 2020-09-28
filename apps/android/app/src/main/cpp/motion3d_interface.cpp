#include <jni.h>
#include <android/log.h>
#include <camera/NdkCameraManager.h>

#include "motion3d_app.h"

Logger LOG("Motion3dJNI");
Motion3dApp *motion3d_app = nullptr;


#define CHECK_MOTION3D_APP(method_name) if (!motion3d_app) { \
    LOG.E(method_name ". Motion3d app is not created"); \
    return; \
}


#include <android/native_window_jni.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <media/NdkImageReader.h>

GLuint createShader(const char *src, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> logStr(maxLength);

        glGetShaderInfoLog(shader, maxLength, &maxLength, logStr.data());
        LOG.E("Could not compile shader %s - %s", src, logStr.data());
    }

    return shader;
}

GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vertexShader);
    glAttachShader(prog, fragmentShader);
    glLinkProgram(prog);

    GLint isLinked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
        LOG.E("Could not link program");

    return prog;
}

void ortho(float *mat4, float left, float right, float bottom, float top, float near, float far) {
    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;

    mat4[0] = 2.0f / rl;
    mat4[12] = -(right + left) / rl;

    mat4[5] = 2.0f / tb;
    mat4[13] = -(top + bottom) / tb;

    mat4[10] = -2.0f / fn;
    mat4[14] = -(far + near) / fn;
}


// Variables for OpenGL setup
GLuint prog;
GLuint vtxShader;
GLuint fragShader;
GLint vtxPosAttrib;
GLint uvsAttrib;
GLint mvpMatrix;
GLint texMatrix;
GLint texSampler;
GLint color;
GLint size;
GLuint buf[2];

// The id is generated in Kotlin and passed to C++
GLuint textureId;

int width = 1080;
int height = 1886;

//static const char *vertexShaderSrc = R"(
//        precision highp float;
//        attribute vec3 vertexPosition;
//        attribute vec2 uvs;
//        varying vec2 varUvs;
//        uniform mat4 texMatrix;
//        uniform mat4 mvp;
//        void main()
//        {
//            varUvs = (texMatrix * vec4(uvs.x, uvs.y, 0, 1.0)).xy;
//            gl_Position = mvp * vec4(vertexPosition, 1.0);
//        }
//)";

//static const char *fragmentShaderSrc = R"(
//        #extension GL_OES_EGL_image_external : require
//        precision mediump float;
//        varying vec2 varUvs;
//        uniform samplerExternalOES texSampler;
//        uniform vec4 color;
//        uniform vec2 size;
//        void main()
//        {
//            if (gl_FragCoord.x/size.x < 0.5) {
//                gl_FragColor = texture2D(texSampler, varUvs) * color;
//            }
//            else {
//                const float threshold = 1.1;
//                vec4 c = texture2D(texSampler, varUvs);
//                if (length(c) > threshold) {
//                    gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
//                } else {
//                    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
//                }
//            }
//        }
//)";

static const char *vertexShaderSrc = R"(
        precision highp float;
        attribute vec3 vertexPosition;
        attribute vec2 uvs;
        varying vec2 varUvs;
        uniform mat4 texMatrix;
        uniform mat4 mvp;
        void main()
        {
//            varUvs = vec4(uvs.x, uvs.y, 0, 1.0)).xy;
            varUvs = (texMatrix * vec4(uvs.x, uvs.y, 0, 1.0)).xy;
            gl_Position = vec4(vertexPosition, 1.0);
        }
)";

static const char *fragmentShaderSrc = R"(
        #extension GL_OES_EGL_image_external : require
        precision mediump float;
        varying vec2 varUvs;
        uniform sampler2D texSampler;
        uniform vec4 color;
        uniform vec2 size;
        void main()
        {
            gl_FragColor = texture2D(texSampler, varUvs);
        }
)";

static ACameraManager *cameraManager = nullptr;

static ACameraDevice *cameraDevice = nullptr;

static ACameraOutputTarget *textureTarget = nullptr;

static ACaptureRequest *request = nullptr;

static ANativeWindow *textureWindow = nullptr;

static ACameraCaptureSession *textureSession = nullptr;

static ACaptureSessionOutput *textureOutput = nullptr;

static ACaptureSessionOutput *output = nullptr;

static ACaptureSessionOutputContainer *outputs = nullptr;

#define WITH_IMAGE_READER

#ifdef WITH_IMAGE_READER
static ANativeWindow* imageWindow = nullptr;

static ACameraOutputTarget* imageTarget = nullptr;

static AImageReader* imageReader = nullptr;

static ACaptureSessionOutput* imageOutput = nullptr;
#endif

AImage *g_image = nullptr;
uint8_t *g_image_data[] = {nullptr, nullptr};
bool g_image_avail[] = {false, false};
int g_image_ind = 0;
int g_image_data_len = 0;
int32_t g_width, g_height;
std::mutex g_image_lock;
bool g_image_first = true;



/**
 * Device listeners
 */

static void onDisconnected(void* context, ACameraDevice* device)
{
    LOG.D("onDisconnected");
}

static void onError(void* context, ACameraDevice* device, int error)
{
    LOG.D("error %d", error);
}

static ACameraDevice_stateCallbacks cameraDeviceCallbacks = {
        .context = nullptr,
        .onDisconnected = onDisconnected,
        .onError = onError,
};

/**
 * Session state callbacks
 */

static void onSessionActive(void *context, ACameraCaptureSession *session) {
    LOG.D("onSessionActive()");
}

static void onSessionReady(void *context, ACameraCaptureSession *session) {
    LOG.D("onSessionReady()");
}

static void onSessionClosed(void *context, ACameraCaptureSession *session) {
    LOG.D("onSessionClosed()");
}

static ACameraCaptureSession_stateCallbacks sessionStateCallbacks{
        .context = nullptr,
        .onClosed = onSessionClosed,
        .onReady = onSessionReady,
        .onActive = onSessionActive
};

/**
 * Image reader setup. If you want to use AImageReader, enable this in CMakeLists.txt.
 */

#ifdef WITH_IMAGE_READER
static void imageCallback(void* context, AImageReader* reader)
{
    AImage *image;
    auto status = AImageReader_acquireNextImage(reader, &image);

//    LOG.D("imageCallback(): %d", status);
    // Check status here ...
    int32_t f;
    AImage_getWidth(image, &g_width);
    AImage_getHeight(image, &g_height);
    AImage_getFormat(image, &f);

    cv::Mat _ndk_rgb_img, _ndk_gray_img;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &uPixel, &uLen);
    AImage_getPlaneData(image, 2, &vPixel, &vLen);

    g_image_lock.lock();
    g_image_data_len = yLen + vLen + uLen;
    if (g_image_data[g_image_ind] == nullptr) {
        g_image_data[g_image_ind] = new uint8_t[g_image_data_len];
    }
    memcpy(g_image_data[g_image_ind], yPixel, yLen);
    memcpy(g_image_data[g_image_ind] + yLen, vPixel, vLen);
    memcpy(g_image_data[g_image_ind] + yLen + vLen, uPixel, uLen);
    g_image_avail[g_image_ind] = true;
    g_image_lock.unlock();

//    LOG.D("Width x Height x Format: %d x %d x %#x. %d", g_width, g_height, f, g_image_data_len);
    AImage_delete(image);

    // Try to process data without blocking the callback
//    std::thread processor([=](){
//
//        uint8_t *data = nullptr;
//        int len = 0;
//        AImage_getPlaneData(image, 0, &data, &len);
//
//        // Process data here
//        // ...
//
//        AImage_delete(image);
//    });
//    processor.detach();
}

AImageReader* createJpegReader()
{
    AImageReader* reader = nullptr;
//    AImageReader_new(1920, 1080, AIMAGE_FORMAT_JPEG,
//                     4, &reader);
    AImageReader_new(1920, 1080, AIMAGE_FORMAT_YUV_420_888,
                     4, &reader);
//    AImageReader_new(1920, 1080, AIMAGE_FORMAT_RGBA_8888,
//                     4, &reader);

    //if (status != AMEDIA_OK)
        // Handle errors here

    AImageReader_ImageListener listener{
            .context = nullptr,
            .onImageAvailable = imageCallback,
    };

    AImageReader_setImageListener(reader, &listener);

    return reader;
}

ANativeWindow* createSurface(AImageReader* reader)
{
    ANativeWindow *nativeWindow;
    AImageReader_getWindow(reader, &nativeWindow);

    return nativeWindow;
}
#endif


/**
 * Capture callbacks
 */

void onCaptureFailed(void *context, ACameraCaptureSession *session,
                     ACaptureRequest *request, ACameraCaptureFailure *failure) {
    LOG.E("onCaptureFailed ");
}

void onCaptureSequenceCompleted(void *context, ACameraCaptureSession *session,
                                int sequenceId, int64_t frameNumber) {}

void onCaptureSequenceAborted(void *context, ACameraCaptureSession *session,
                              int sequenceId) {}

void onCaptureCompleted(
        void *context, ACameraCaptureSession *session,
        ACaptureRequest *request, const ACameraMetadata *result) {
//    LOG.D("Capture completed");
}

static ACameraCaptureSession_captureCallbacks captureCallbacks{
        .context = nullptr,
        .onCaptureStarted = nullptr,
        .onCaptureProgressed = nullptr,
        .onCaptureCompleted = onCaptureCompleted,
        .onCaptureFailed = onCaptureFailed,
        .onCaptureSequenceCompleted = onCaptureSequenceCompleted,
        .onCaptureSequenceAborted = onCaptureSequenceAborted,
        .onCaptureBufferLost = nullptr,
};

std::string getBackFacingCamId(ACameraManager *cameraManager) {
    ACameraIdList *cameraIds = nullptr;
    ACameraManager_getCameraIdList(cameraManager, &cameraIds);

    std::string backId;

    LOG.D("found camera count %d", cameraIds->numCameras);

    for (int i = 0; i < cameraIds->numCameras; ++i) {
        const char *id = cameraIds->cameraIds[i];

        ACameraMetadata *metadataObj;
        ACameraManager_getCameraCharacteristics(cameraManager, id, &metadataObj);

        ACameraMetadata_const_entry lensInfo = {0};
        ACameraMetadata_getConstEntry(metadataObj, ACAMERA_LENS_FACING, &lensInfo);

        auto facing = static_cast<acamera_metadata_enum_android_lens_facing_t>(
                lensInfo.data.u8[0]);

        // Found a back-facing camera?
        if (facing == ACAMERA_LENS_FACING_BACK) {
            backId = id;
            break;
        }
    }

    ACameraManager_deleteCameraIdList(cameraIds);

    return backId;
}

void printCamProps(ACameraManager *cameraManager, const char *id)
{
    // exposure range
    ACameraMetadata *metadataObj;
    ACameraManager_getCameraCharacteristics(cameraManager, id, &metadataObj);

    ACameraMetadata_const_entry entry = {0};
    ACameraMetadata_getConstEntry(metadataObj,
                                  ACAMERA_SENSOR_INFO_EXPOSURE_TIME_RANGE, &entry);

    int64_t minExposure = entry.data.i64[0];
    int64_t maxExposure = entry.data.i64[1];
    LOG.D("camProps: minExposure=%lld vs maxExposure=%lld", minExposure, maxExposure);
    ////////////////////////////////////////////////////////////////

    // sensitivity
    ACameraMetadata_getConstEntry(metadataObj,
                                  ACAMERA_SENSOR_INFO_SENSITIVITY_RANGE, &entry);

    int32_t minSensitivity = entry.data.i32[0];
    int32_t maxSensitivity = entry.data.i32[1];

    LOG.D("camProps: minSensitivity=%d vs maxSensitivity=%d", minSensitivity, maxSensitivity);
    ////////////////////////////////////////////////////////////////

    // JPEG format
    ACameraMetadata_getConstEntry(metadataObj,
                                  ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);

    for (int i = 0; i < entry.count; i += 4)
    {
        // We are only interested in output streams, so skip input stream
        int32_t input = entry.data.i32[i + 3];
        if (input)
            continue;

        int32_t format = entry.data.i32[i + 0];
        if (format == AIMAGE_FORMAT_JPEG)
        {
            int32_t width = entry.data.i32[i + 1];
            int32_t height = entry.data.i32[i + 2];
            LOG.D("camProps: maxWidth=%d vs maxHeight=%d", width, height);
        }
    }

    // cam facing
    ACameraMetadata_getConstEntry(metadataObj,
                                  ACAMERA_SENSOR_ORIENTATION, &entry);

    int32_t orientation = entry.data.i32[0];
    LOG.D("camProps: %d", orientation);
}


/**
 * Functions used to set-up the camera and draw
 * camera frames into SurfaceTexture
 */

static void initCam()
{
    cameraManager = ACameraManager_create();

    auto id = getBackFacingCamId(cameraManager);
    ACameraManager_openCamera(cameraManager, id.c_str(), &cameraDeviceCallbacks, &cameraDevice);

    printCamProps(cameraManager, id.c_str());
}

static void exitCam()
{
    if (cameraManager)
    {
        // Stop recording to SurfaceTexture and do some cleanup
        ACameraCaptureSession_stopRepeating(textureSession);
        ACameraCaptureSession_close(textureSession);
        ACaptureSessionOutputContainer_free(outputs);
        ACaptureSessionOutput_free(output);

        ACameraDevice_close(cameraDevice);
        ACameraManager_delete(cameraManager);
        cameraManager = nullptr;

#ifdef WITH_IMAGE_READER
        AImageReader_delete(imageReader);
        imageReader = nullptr;
#endif

        // Capture request for SurfaceTexture
        ANativeWindow_release(textureWindow);
        ACaptureRequest_free(request);
    }
}

static void initCam(JNIEnv* env, jobject surface)
{
    // Prepare surface
    LOG.D("ANativeWindow_fromSurface");
    textureWindow = ANativeWindow_fromSurface(env, surface);

    // Prepare request for texture target
    LOG.D("ACameraDevice_createCaptureRequest: %d", cameraDevice);
    ACameraDevice_createCaptureRequest(cameraDevice, TEMPLATE_PREVIEW, &request);

    // Prepare outputs for session
    int32_t format = ANativeWindow_getFormat(textureWindow);
//    ANativeWindow_setBuffersGeometry(textureWindow, 1920, 1080, format);
    ACaptureSessionOutput_create(textureWindow, &textureOutput);
    ACaptureSessionOutputContainer_create(&outputs);
//    ACaptureSessionOutputContainer_add(outputs, textureOutput);

// Enable ImageReader example in CMakeLists.txt. This will additionally
// make image data available in imageCallback().
#ifdef WITH_IMAGE_READER
    imageReader = createJpegReader();
    imageWindow = createSurface(imageReader);
    ANativeWindow_acquire(imageWindow);
    ACameraOutputTarget_create(imageWindow, &imageTarget);
    ACaptureRequest_addTarget(request, imageTarget);
    ACaptureSessionOutput_create(imageWindow, &imageOutput);
    ACaptureSessionOutputContainer_add(outputs, imageOutput);
#endif

    // Prepare target surface
//    ANativeWindow_acquire(textureWindow);
//    ACameraOutputTarget_create(textureWindow, &textureTarget);
//    ACaptureRequest_addTarget(request, textureTarget);

    // Create the session
    ACameraDevice_createCaptureSession(cameraDevice, outputs, &sessionStateCallbacks, &textureSession);

    // Start capturing continuously
    ACameraCaptureSession_setRepeatingRequest(textureSession, &captureCallbacks, 1, &request, nullptr);
}

static void initSurface(JNIEnv* env, jint texId, jobject surface)
{
    // Init shaders
    vtxShader = createShader(vertexShaderSrc, GL_VERTEX_SHADER);
    fragShader = createShader(fragmentShaderSrc, GL_FRAGMENT_SHADER);
    prog = createProgram(vtxShader, fragShader);

    // Store attribute and uniform locations
    vtxPosAttrib = glGetAttribLocation(prog, "vertexPosition");
    uvsAttrib = glGetAttribLocation(prog, "uvs");
    mvpMatrix = glGetUniformLocation(prog, "mvp");
    texMatrix = glGetUniformLocation(prog, "texMatrix");
    texSampler = glGetUniformLocation(prog, "texSampler");
    color = glGetUniformLocation(prog, "color");
    size = glGetUniformLocation(prog, "size");

    // Prepare buffers
    glGenBuffers(2, buf);

    // Set up vertices
    float vertices[] {
            // x, y, z, u, v
            -1, -1, 0, 0, 0,
            -1, 1, 0, 0, 1,
            1, 1, 0, 1, 1,
            1, -1, 0, 1, 0
    };
    glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    // Set up indices
    GLuint indices[] { 2, 1, 0, 0, 3, 2 };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

    // We can use the id to bind to GL_TEXTURE_EXTERNAL_OES
//    textureId = texId;
    glGenTextures(1, &textureId);
    // Prepare the surfaces/targets & initialize session
    initCam(env, surface);
}

void checkGLError()
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
//        GL_NO_ERROR
        LOG.E("GL Err: %#06x", err);
    }
}
#include <opencv2/core.hpp>

static void drawFrame(JNIEnv* env, jfloatArray texMatArray)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0, 0, 0, 1);

    glUseProgram(prog);

    glViewport(0, 0, width, height);
    // Configure main transformations
    float mvp[] = {
            1.0f, 0, 0, 0,
            0, 1.0f, 0, 0,
            0, 0, 1.0f, 0,
            0, 0, 0, 1.0f
    };

    float aspect = width > height ? float(width)/float(height) : float(height)/float(width);
    if (width < height) // portrait
        ortho(mvp, -1.0f, 1.0f, -aspect, aspect, -1.0f, 1.0f);
    else // landscape
        ortho(mvp, -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);

    glUniformMatrix4fv(mvpMatrix, 1, false, mvp);


    // Prepare texture for drawing

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform1i(texSampler, 0);

    if (g_image_avail[g_image_ind]) {
        g_image_lock.lock();
        uint8_t *data = g_image_data[g_image_ind];
        g_image_avail[g_image_ind] = false;
        g_image_ind = (g_image_ind + 1) % 2;
        g_image_lock.unlock();

//        LOG.D("cv w x h: %d x %d", g_width, g_height);
//        if (g_image_first) {
//            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, g_width, g_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
//            g_image_first = false;
//        } else {
//            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_width, g_height, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
//        }

        cv::Mat mYUV = cv::Mat(g_height, g_width + g_width / 2, CV_8UC1, data);
//        cv::Mat mYUV = cv::Mat(g_width + g_width / 2, g_height, CV_8UC1, data);
        cv::Mat img_rgb;
        cv::cvtColor(mYUV, img_rgb, cv::COLOR_YUV2RGB_NV21, 3);
        cv::rotate(img_rgb, img_rgb, cv::ROTATE_90_CLOCKWISE);
        LOG.D("cv w x h: %d x %d. t: %d", img_rgb.cols, img_rgb.rows, img_rgb.type());
        if (g_image_first) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_rgb.cols, img_rgb.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, img_rgb.data);
            g_image_first = false;
        } else {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img_rgb.cols, img_rgb.rows, GL_RGB, GL_UNSIGNED_BYTE, img_rgb.data);
        }

        checkGLError();
    }


    // Pass SurfaceTexture transformations to shader
    float* tm = env->GetFloatArrayElements(texMatArray, 0);
//    LOG.D("tm: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f", tm[0], tm[1], tm[2], tm[3], tm[4], tm[5], tm[6], tm[7]);
    glUniformMatrix4fv(texMatrix, 1, false, tm);
    env->ReleaseFloatArrayElements(texMatArray, tm, 0);

    // I use red color to mix with camera frames
    float c[] = { 1, 0, 0, 1 };
    glUniform4fv(color, 1, (GLfloat*)c);

    // Size of the window is used in fragment shader
    // to split the window
    float sz[2] = {0};
    sz[0] = width;
    sz[1] = height;
    glUniform2fv(size, 1, (GLfloat*)sz);

    // Set up vertices/indices and draw
    glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf[1]);

    glEnableVertexAttribArray(vtxPosAttrib);
    glVertexAttribPointer(vtxPosAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);

    glEnableVertexAttribArray(uvsAttrib);
    glVertexAttribPointer(uvsAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void *)(3 * sizeof(float)));


    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_create(JNIEnv *env, jobject thiz) {
    LOG.D("Java_ai_motion3d_Motion3d_create");
    if (!motion3d_app) {
        motion3d_app = new Motion3dApp();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_destroy(JNIEnv *env, jobject thiz) {
    LOG.D("Java_ai_motion3d_Motion3d_destroy");
    delete motion3d_app;
    motion3d_app = nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_startCamera(JNIEnv *env, jobject thiz) {
    CHECK_MOTION3D_APP("Java_ai_motion3d_Motion3d_startCamera")
    LOG.D("Java_ai_motion3d_Motion3d_startCamera");
//    motion3d_app->startCamera();
    initCam();
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_stopCamera(JNIEnv *env, jobject thiz) {
    CHECK_MOTION3D_APP("Java_ai_motion3d_Motion3d_stopCamera")
    LOG.D("Java_ai_motion3d_Motion3d_stopCamera");
//    motion3d_app->stopCamera();
    exitCam();
}


extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_CameraRenderer_onSurfaceCreated(JNIEnv *env, jobject thiz, jint texture_id,
                                                 jobject surface) {
    LOG.D("Java_ai_motion3d_CameraRenderer_onSurfaceCreated");
    initSurface(env, texture_id, surface);
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_CameraRenderer_onSurfaceChanged(JNIEnv *env, jobject thiz, jint w,
                                                 jint h) {
    LOG.D("Java_ai_motion3d_CameraRenderer_onSurfaceChanged (w x h): %d x %d", w, h);
    width = w;
    height = h;
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_CameraRenderer_onDrawFrame(JNIEnv *env, jobject thiz, jfloatArray tex_mat) {
    drawFrame(env, tex_mat);
}
