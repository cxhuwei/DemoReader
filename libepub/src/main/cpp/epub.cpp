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
JNIEXPORT jobject JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_openDocumentWithPath(JNIEnv *env, jclass type,
                                                                   jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    // TODO

    env->ReleaseStringUTFChars(path_, path);
}