#pragma once

#include <jni.h>

class android_os_Message {
public:
    static void init(JNIEnv *env);

    static android_os_Message create();

    void set_arg1(int value);
    void set_arg2(int value);

private:
    android_os_Message(jobject obj);

    static JNIEnv *env_;
    static jclass cls_;
    static jmethodID ctor_;
    static jfieldID arg1_field_id;
    static jfieldID arg2_field_id;

    jobject obj_;
};

