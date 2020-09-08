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
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_create(JNIEnv *env, jobject thiz) {
    if (!motion3d_app) {
        motion3d_app = new Motion3dApp();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_destroy(JNIEnv *env, jobject thiz) {
    delete motion3d_app;
    motion3d_app = nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_startCamera(JNIEnv *env, jobject thiz) {
    if (!motion3d_app) {
        __android_log_write(ANDROID_LOG_ERROR, gTag, "startCamera le. Motion3d app is not created");
        return;
    }
    motion3d_app->startCamera();
}

extern "C"
JNIEXPORT void JNICALL
Java_ai_motion3d_Motion3d_stopCamera(JNIEnv *env, jobject thiz) {
    if (!motion3d_app) {
        __android_log_write(ANDROID_LOG_ERROR, gTag, "stopCamera le. Motion3d app is not created");
        return;
    }
    motion3d_app->stopCamera();
}

