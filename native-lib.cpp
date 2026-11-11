#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_cpptest_NativeHelper_getHello(JNIEnv* env, jclass clazz) {
    std::string hello = "ПЕРЕМОГА! C++ працює!";
    return env->NewStringUTF(hello.c_str());
}
