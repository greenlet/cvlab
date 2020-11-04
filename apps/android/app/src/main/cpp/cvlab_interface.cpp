#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <android/native_window_jni.h>
#include <camera/NdkCameraManager.h>
#include <jni.h>
#include <media/NdkImageReader.h>

#include "logger.h"
#include "cvlab_app.h"

static Logger LOG("CvlabJNI");
static CvlabApp *cvlab_app = nullptr;

#define CHECK_CVLAB_APP(method_name)                     \
    if (!cvlab_app) {                                    \
        LOG.E(method_name ". Cvlab app is not created"); \
        return;                                             \
    }

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_create(JNIEnv *env, jobject thiz) {
    LOG.D("Java_ai_cvlab_Cvlab_create");
    if (!cvlab_app) {
        cvlab_app = new CvlabApp();
    }
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_destroy(JNIEnv *env, jobject thiz) {
    LOG.D("Java_ai_cvlab_Cvlab_destroy");
    delete cvlab_app;
    cvlab_app = nullptr;
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_startCamera(JNIEnv *env, jobject thiz) {
    CHECK_CVLAB_APP("Java_ai_cvlab_Cvlab_startCamera")
    LOG.D("Java_ai_cvlab_Cvlab_startCamera");
    cvlab_app->camera().start();
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_Cvlab_stopCamera(JNIEnv *env, jobject thiz) {
    CHECK_CVLAB_APP("Java_ai_cvlab_Cvlab_stopCamera")
    LOG.D("Java_ai_cvlab_Cvlab_stopCamera");
    cvlab_app->camera().stop();
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_CameraRenderer_onSurfaceCreated(JNIEnv *env,
                                                                                   jobject thiz) {
    LOG.D("Java_ai_cvlab_CameraRenderer_onSurfaceCreated");
    if (!cvlab_app) {
        LOG.W("Cvlab App is not started");
        return;
    }
    cvlab_app->renderer().init();
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_CameraRenderer_onSurfaceChanged(JNIEnv *env,
                                                                                   jobject thiz,
                                                                                   jint w, jint h) {
    LOG.D("Java_ai_cvlab_CameraRenderer_onSurfaceChanged (w x h): %d x %d", w, h);
    if (!cvlab_app) {
        LOG.W("Cvlab 3d App is not started");
        return;
    }
    cvlab_app->renderer().updateSize(w, h);
}

extern "C" JNIEXPORT void JNICALL Java_ai_cvlab_CameraRenderer_onDrawFrame(JNIEnv *env,
                                                                              jobject thiz) {
    if (!cvlab_app) {
        return;
    }
    cvlab_app->renderer().drawFrame();
}
