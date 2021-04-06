#include "android_os_message.h"

JNIEnv *android_os_Message::env_;
jclass android_os_Message::cls_;
jmethodID android_os_Message::ctor_;
jfieldID android_os_Message::arg1_field_id;
jfieldID android_os_Message::arg2_field_id;

void android_os_Message::init(JNIEnv *env) {
    env_ = env;
    cls_ = env_->FindClass("android/os/Message");
    ctor_ = env_->GetMethodID(cls_, "<init>", "()V");
    arg1_field_id = env_->GetFieldID(cls_, "arg1", "I");
    arg2_field_id = env_->GetFieldID(cls_, "arg2", "I");
}

android_os_Message android_os_Message::create() {
    jobject obj = env_->NewObject(cls_, ctor_);
    return android_os_Message(obj);
}

android_os_Message::android_os_Message(jobject obj) : obj_(obj) {}

void android_os_Message::set_arg1(int value) {
    env_->SetIntField(obj_, arg1_field_id, value);
}

void android_os_Message::set_arg2(int value) {
    env_->SetIntField(obj_, arg2_field_id, value);
}

