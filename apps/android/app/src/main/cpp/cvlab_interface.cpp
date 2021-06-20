#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <android/native_window_jni.h>
#include <camera/NdkCameraManager.h>
#include <jni.h>
#include <media/NdkImageReader.h>

#include "cvlab_app.h"
#include "logger.h"
//#include "java_classes/java_classes.h"

Logger Log("CvlabJNI");
std::mutex cvlab_app_mutex;
static CvlabApp *cvlab_app = nullptr;

#define ACQUIRE_CVLAB_APP_                  \
    std::lock_guard<std::mutex> lock(cvlab_app_mutex);   \
    if (!cvlab_app) {                                    \
        Log.E("%s. Cvlab app is not created", __FUNCTION__); \
        return;                                          \
    }

#define ACQUIRE_CVLAB_APP \
    ACQUIRE_CVLAB_APP_  \
    Log.D(__FUNCTION__);

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_create(JNIEnv *env, jobject thiz) {
    Log.D(__FUNCTION__);
    std::lock_guard<std::mutex> lock(cvlab_app_mutex);
    if (!cvlab_app) {
        cvlab_app = new CvlabApp(env, thiz);
//        initJniHelpers(env);
    }
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_destroy(JNIEnv *env, jobject thiz) {
    Log.D(__FUNCTION__);
    std::lock_guard<std::mutex> lock(cvlab_app_mutex);
    delete cvlab_app;
    cvlab_app = nullptr;
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_startCamera(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP
    cvlab_app->camera().start();
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_stopCamera(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP
    cvlab_app->camera().stop();
    cvlab_app->stopCalibration();
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_CameraRenderer_onSurfaceCreated(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP
    cvlab_app->renderer()->init_render();
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_CameraRenderer_onSurfaceChanged(JNIEnv *env, jobject thiz, jint w, jint h) {
    ACQUIRE_CVLAB_APP_
    Log.D("Java_ai_cvlab_CameraRenderer_onSurfaceChanged (w x h): %d x %d", w, h);
    cvlab_app->renderer()->updateSize_render(w, h);
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_CameraRenderer_onDrawFrame(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP_
    cvlab_app->renderer()->drawFrame_render();
}

extern "C" JNIEXPORT void JNICALL
Java_ai_cvlab_Cvlab_startCalibration_1jni(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP
    cvlab_app->startCalibration(env, thiz);
}

extern "C" JNIEXPORT void JNICALL
Java_ai_cvlab_Cvlab_stopCalibration_1jni(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP
    cvlab_app->stopCalibration();
}

extern "C" JNIEXPORT void JNICALL
Java_ai_cvlab_Cvlab_startCalibCapture(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP
    cvlab_app->startCalibCapture(env, thiz);
}

extern "C" JNIEXPORT void JNICALL
Java_ai_cvlab_Cvlab_stopCalibCapture(JNIEnv *env, jobject thiz) {
    ACQUIRE_CVLAB_APP
    cvlab_app->stopCalibCapture();
}

extern "C" JNIEXPORT void JNICALL
Java_ai_cvlab_Cvlab_startCalibCalc(JNIEnv *env, jobject thiz) {
    cvlab_app->startCalibCalc();
}

extern "C" JNIEXPORT void JNICALL
Java_ai_cvlab_Cvlab_stopCalibCalc(JNIEnv *env, jobject thiz) {
    cvlab_app->stopCalibCalc();
}