//
// Created by huwei on 2018/5/16.
//

#include <jni.h>
#include <string>
#include<android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "EPUB_READER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG ,__VA_ARGS__)
#endif

extern "C"
JNIEXPORT jint JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeSetForegroundColor(JNIEnv *env,
                                                                       jobject instance,
                                                                       jint color) {

    // TODO
    return 1;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeSetBackgroundColor(JNIEnv *env,
                                                                       jobject instance,
                                                                       jint color) {

    // TODO
    return 1;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeSetFontResource(JNIEnv *env, jobject instance,
                                                                    jobjectArray fontResource) {

    // TODO
    return 1;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeSetTextLevel(JNIEnv *env, jobject instance,
                                                                 jint level) {

    // TODO
    return 1;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeLayout(JNIEnv *env, jobject instance,
                                                           jint width, jint height, jint left,
                                                           jint top, jint right, jint bottom,
                                                           jfloat density) {

    // TODO
    return 1;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeOpenDocument(JNIEnv *env, jobject instance,
                                                                 jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    // TODO

    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeGetCatalog(JNIEnv *env, jobject instance) {

    // TODO

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeGetFileCount(JNIEnv *env, jobject instance) {

    // TODO

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeGetPageCountByFile(JNIEnv *env,
                                                                       jobject instance,
                                                                       jint fileId) {

    // TODO

}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeLocateCatalog(JNIEnv *env, jobject instance,
                                                                  jint index) {

    // TODO

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeDrawPage(JNIEnv *env, jobject instance,
                                                             jint fileId, jint pageNumber,
                                                             jobject bitmap) {

    // TODO

}

extern "C"
JNIEXPORT void JNICALL
Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeCloseDocument(JNIEnv *env, jobject instance) {
    LOGI("nativeCloseDocument()");
    // TODO

}