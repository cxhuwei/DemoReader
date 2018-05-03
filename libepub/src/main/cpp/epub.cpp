#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring

JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_sayHello(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
