#include <jni.h>
#include <android/log.h>

#include "motion3d_app.h"

Motion3dApp *motion3d_app = nullptr;

inline jlong n2j(Motion3dApp *motion3d_app) {
    return reinterpret_cast<intptr_t>(motion3d_app);
}

inline Motion3dApp *j2n(jlong ptr) {
    return reinterpret_cast<Motion3dApp *>(ptr);
}

const char *gTag = "Motion3dInterface";

extern "C"
JNIEXPORT jlong JNICALL
Java_ai_motion3d_Motion3d_create(JNIEnv *env, jobject thiz) {
    __android_log_write(ANDROID_LOG_DEBUG, gTag, "create");
    if (!motion3d_app) {
        motion3d_app = new Motion3dApp();
    }
    return n2j(motion3d_app);
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_destroy(JNIEnv *env, jobject thiz, jlong native_app) {
    __android_log_write(ANDROID_LOG_DEBUG, gTag, "destroy");
    delete motion3d_app;
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_startCamera(JNIEnv *env, jobject thiz, jlong native_app) {
    __android_log_write(ANDROID_LOG_DEBUG, gTag, "startCamera");
    Motion3dApp *motion3d_app = j2n(native_app);
    motion3d_app->startCamera();
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_stopCamera(JNIEnv *env, jobject thiz, jlong native_app) {
    __android_log_write(ANDROID_LOG_DEBUG, gTag, "stopCamera");
    Motion3dApp *motion3d_app = j2n(native_app);
    motion3d_app->stopCamera();
}

