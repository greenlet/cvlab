#include <jni.h>
#include <camera/NdkCameraManager.h>
#include <android/native_window_jni.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <media/NdkImageReader.h>

#include "motion3d_app.h"
#include "logger.h"

static Logger LOG("Motion3dJNI");
static Motion3dApp *motion3d_app = nullptr;

#define CHECK_MOTION3D_APP(method_name) if (!motion3d_app) { \
    LOG.E(method_name ". Motion3d app is not created"); \
    return; \
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
  motion3d_app->camera().start();
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_stopCamera(JNIEnv *env, jobject thiz) {
  CHECK_MOTION3D_APP("Java_ai_motion3d_Motion3d_stopCamera")
  LOG.D("Java_ai_motion3d_Motion3d_stopCamera");
  motion3d_app->camera().stop();
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_CameraRenderer_onSurfaceCreated(JNIEnv *env, jobject thiz) {
  LOG.D("Java_ai_motion3d_CameraRenderer_onSurfaceCreated");
  motion3d_app->renderer().init();
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_CameraRenderer_onSurfaceChanged(JNIEnv *env, jobject thiz, jint w,
                                                 jint h) {
  LOG.D("Java_ai_motion3d_CameraRenderer_onSurfaceChanged (w x h): %d x %d", w, h);
  motion3d_app->renderer().updateSize(w, h);
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_CameraRenderer_onDrawFrame(JNIEnv *env, jobject thiz) {
  motion3d_app->renderer().drawFrame();
}
