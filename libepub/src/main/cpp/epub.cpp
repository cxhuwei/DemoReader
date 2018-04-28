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

extern "C"
JNIEXPORT jint JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_openDocument(JNIEnv *env, jclass type,
                                                           jstring path_) {
    return 0;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_getTitle(JNIEnv *env, jclass type) {
    return NULL;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_getAuthor(JNIEnv *env, jclass type) {
    return NULL;
}

