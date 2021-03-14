#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <android/native_window_jni.h>
#include <camera/NdkCameraManager.h>
#include <jni.h>
#include <media/NdkImageReader.h>

#include "cvlab_app.h"
#include "logger.h"

static Logger LOG("CvlabJNI");
std::mutex cvlab_app_mutex;
static CvlabApp *cvlab_app = nullptr;

#define ACQUIRE_CVLAB_APP_(method_name)                  \
    std::lock_guard<std::mutex> lock(cvlab_app_mutex);   \
    if (!cvlab_app) {                                    \
        LOG.E(method_name ". Cvlab app is not created"); \
        return;                                          \
    }

#define ACQUIRE_CVLAB_APP(method_name) \
    ACQUIRE_CVLAB_APP_(method_name)    \
    LOG.D(method_name);

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_create(JNIEnv *env, jobject thiz) {
    LOG.D("Java_ai_cvlab_Cvlab_create");
    std::lock_guard<std::mutex> lock(cvlab_app_mutex);
    if (!cvlab_app) {
        cvlab_app = new CvlabApp();
    }
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_destroy(JNIEnv *env, jobject thiz) {
    LOG.D("Java_ai_cvlab_Cvlab_destroy");
    std::lock_guard<std::mutex> lock(cvlab_app_mutex);
    delete cvlab_app;
    cvlab_app = nullptr;
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_startCamera(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP("Java_ai_cvlab_Cvlab_startCamera")
    cvlab_app->camera().start();
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_stopCamera(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP("Java_ai_cvlab_Cvlab_stopCamera")
    cvlab_app->camera().stop();
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_CameraRenderer_onSurfaceCreated(JNIEnv *env,
                                                                                jobject thiz) {
    ACQUIRE_CVLAB_APP("Java_ai_cvlab_CameraRenderer_onSurfaceCreated")
    cvlab_app->renderer().init();
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_CameraRenderer_onSurfaceChanged(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jint w, jint h) {
    ACQUIRE_CVLAB_APP_("Java_ai_cvlab_CameraRenderer_onSurfaceChanged")
    LOG.D("Java_ai_cvlab_CameraRenderer_onSurfaceChanged (w x h): %d x %d", w, h);
    cvlab_app->renderer().updateSize(w, h);
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_CameraRenderer_onDrawFrame(JNIEnv *env,
                                                                           jobject thiz) {
    ACQUIRE_CVLAB_APP_("Java_ai_cvlab_CameraRenderer_onDrawFrame")
    cvlab_app->renderer().drawFrame();
}
