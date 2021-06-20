#include "jni_helpers.h"
#include "android_os_message.h"

static JavaVM *java_vm = nullptr;
static thread_local JNIEnv *jni_env = nullptr;


void initJniHelpers(JNIEnv *env) {
    jni_env = env;
    jni_env->GetJavaVM(&java_vm);
}

JNIEnv *getJniEvn() {
    if (jni_env == nullptr) {
        java_vm->GetEnv(reinterpret_cast<void **>(&jni_env), JNI_VERSION_1_6);
    }
    return jni_env;
}
