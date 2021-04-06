#include "java_classes.h"
#include "android_os_message.h"

void initJavaClasses(JNIEnv *env) {
    android_os_Message::init(env);
}
