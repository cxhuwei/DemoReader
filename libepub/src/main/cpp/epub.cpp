#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <time.h>
#include  <unistd.h>

#include "SZEbookReader.h"
#include<android/bitmap.h>

#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "EPUB_PARSER"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG ,__VA_ARGS__)
#endif

#define RGBA_A(p) (((p) & 0xFF000000) >> 24)
#define RGBA_R(p) (((p) & 0x00FF0000) >> 16)
#define RGBA_G(p) (((p) & 0x0000FF00) >>  8)
#define RGBA_B(p)  ((p) & 0x000000FF)
#define MAKE_RGBA(r, g, b, a) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

typedef enum _GET_JNIENV_STATUS {
    GET_FAIL = 0,
    GET_SUCCES_NOATTACH,
    GET_SUCCES_ATTCH,
} Get_JNIEnv_Status;

SZEbookReader *g_BookReader = NULL;
JavaVM *g_jvm = NULL;
jobject g_obj = NULL;

int g_status = 0;

#pragma mark -- globalfunction

Get_JNIEnv_Status getJNIEnv(JNIEnv **env) {
    Get_JNIEnv_Status GetStatus = GET_FAIL;
    if (g_jvm == NULL) {
        LOGI("getJNIEnv | g_jvm == NULL ");
        return GetStatus;
    }

    int status = g_jvm->GetEnv((void **) env, JNI_VERSION_1_6);
    if (status < 0) {
        LOGI("callback_handler:failed to get JNI environment assuming native thread");
        status = g_jvm->AttachCurrentThread(env, NULL);
        if (status < 0) {
            LOGI("callback_handler: failed to attach current thread");
            LOGI("%s: AttachCurrentThread() failed", __FUNCTION__);
            return GetStatus;
        }
        GetStatus = GET_SUCCES_ATTCH;
    } else {
        GetStatus = GET_SUCCES_NOATTACH;
    }
    return GetStatus;
}

extern int myWcslen(wchar_t *str);

jstring w2js(JNIEnv *env, wchar_t *str) {
    //size_t len = wcslen(str);//
    int len = myWcslen(str);
    // LOGI("EpubParser_getCatalog2Obj: len =%d, ilen =%d", len);
    jchar *str2 = (jchar *) malloc(sizeof(jchar) * (len + 1));
    int i;
    for (i = 0; i < len; i++)
        str2[i] = str[i];
    str2[len] = 0;
    jstring js = env->NewString(str2, len);
    free(str2);
    return js;
}

SZEbookReader *GetBookReader(JNIEnv *env, jobject obj) {
    if (g_BookReader == NULL) {
        g_BookReader = new SZEbookReader();

        env->GetJavaVM(&g_jvm);
        g_obj = env->NewGlobalRef(obj);
    }
    return g_BookReader;
}

void ReleaseBookReader() {
    if (g_BookReader != NULL) {
        delete g_BookReader;
        g_BookReader = NULL;
    }
}

#pragma mark -- globalfunction  end

void DoConverCallback(void *data, char *str, long status) {
    JNIEnv *env = NULL;
    Get_JNIEnv_Status envstatus = getJNIEnv(&env);
    if (envstatus == GET_FAIL) {
        return;
    }
    if (env) {
        g_status = status;
        jclass jEpubDocumentClass = env->GetObjectClass(g_obj);
        if (jEpubDocumentClass == NULL) {
            LOGI("DoConverCallback FindClass() EpubDocument Error.....");
            if (status == GET_SUCCES_ATTCH) {
                g_jvm->DetachCurrentThread();
            }
            return;
        }
        LOGI("DoConverCallback | str:%s  | status:%d", str, status);
        jmethodID javaOnEventMethodID = env->GetMethodID(jEpubDocumentClass, "onEvent",
                                                         "(ILjava/lang/String;)V");
        env->CallVoidMethod(g_obj, javaOnEventMethodID, status, env->NewStringUTF(str));

        env->DeleteLocalRef(jEpubDocumentClass);
    }

    if (envstatus == GET_SUCCES_ATTCH) {
        g_jvm->DetachCurrentThread();
    }
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeSetForegroundColor
 * Signature: (I)I
 */
extern "C"
{
JNIEXPORT jint JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeSetForegroundColor
        (JNIEnv *env, jobject obj, jint color) {
    SZEbookReader *bookReader = GetBookReader(env, obj);

    myRGBQUAD clr;
    clr.rgbBlue = color & 0xff;
    clr.rgbGreen = (color >> 8) & 0xff;
    clr.rgbRed = (color >> 16) & 0xff;
    clr.rgbReserved = (color >> 24) & 0xff;
    bookReader->SetClrForeG(clr);
    return 0;
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeSetBackgroundColor
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeSetBackgroundColor
        (JNIEnv *env, jobject obj, jint color) {
    SZEbookReader *bookReader = GetBookReader(env, obj);

    myRGBQUAD clr;
    clr.rgbBlue = color & 0xff;
    clr.rgbGreen = (color >> 8) & 0xff;
    clr.rgbRed = (color >> 16) & 0xff;
    clr.rgbReserved = (color >> 24) & 0xff;
    bookReader->SetClrBackG(clr);
    return 0;
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeSetFontResource
 * Signature: ([Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeSetFontResource
        (JNIEnv *env, jobject obj, jobjectArray fontArray) {

    jint reNum = 0;
    jint size = env->GetArrayLength(fontArray);
    if (size != 2) {
        return 1;
    }

    SZEbookReader *bookReader = GetBookReader(env, obj);

    jstring ChineseFontPath = (jstring) env->GetObjectArrayElement(fontArray, 0);
    jstring EnglishFontPath = (jstring) env->GetObjectArrayElement(fontArray, 1);

    const char *c_szChineseFontPath = env->GetStringUTFChars(ChineseFontPath, 0);
    const char *c_szEnglishFontPath = env->GetStringUTFChars(EnglishFontPath, 0);

    const char *cp = strrchr(c_szChineseFontPath, '/');
    if (cp) {
        bool re = bookReader->SetChineseFont(cp, c_szChineseFontPath);
        if (!re) {
            reNum = 4;
        }
    } else {
        reNum = 2;
    }

    const char *ep = strrchr(c_szEnglishFontPath, '/');
    if (ep) {
        bool re = bookReader->SetEnglishFont(ep, c_szEnglishFontPath);
        if (!re) {
            reNum = 5;
        }
    } else {
        reNum = 3;
    }

    env->ReleaseStringUTFChars(ChineseFontPath, c_szChineseFontPath);
    env->ReleaseStringUTFChars(EnglishFontPath, c_szEnglishFontPath);
    return reNum;
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeSetTextLevel
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeSetTextLevel
        (JNIEnv *env, jobject obj, jint zoom) {
    SZEbookReader *bookReader = GetBookReader(env, obj);

    return bookReader->SetZoomRatio(zoom) ? 0 : 1;
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeLayout
 * Signature: (IIIIIIF)Z
 */
JNIEXPORT jint JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeLayout
        (JNIEnv *env, jobject obj, jint width, jint height, jint mrginLeft, jint mrginTop,
         jint mrginRight, jint mrginBottom, jfloat density) {
    SZEbookReader *bookReader = GetBookReader(env, obj);

    bookReader->SetMrginLeft(mrginLeft);
    bookReader->SetMrginRight(width-mrginRight);
    bookReader->SetMrginTop(mrginTop);
    bookReader->SetMrginBottom(height -mrginBottom);

    bookReader->Init(width, height, density == 1.0 ? NormalScreen : HDScreen);
    if (density != 1.0) {
        bookReader->SetScreenRatio(density);
    }

    return 0;
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeOpenDocument
 * Signature: (Ljava/lang/String;)Lcom/chaoxing/epub/nativeapi/EpubInfo;
 */
JNIEXPORT jobject JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeOpenDocument
        (JNIEnv *env, jobject obj, jstring bookPath) {
    SZEbookReader *bookReader = GetBookReader(env, obj);


    bookReader->SetCallBackFunction(NULL, DoConverCallback);

    const char *c_bookPath = env->GetStringUTFChars(bookPath, 0);
    bool re = bookReader->OpenEbook(c_bookPath);
    env->ReleaseStringUTFChars(bookPath, c_bookPath);
    if (re) {
        jclass jEpubInfoClass = env->FindClass("com/chaoxing/epub/nativeapi/EpubInfo");
        if (jEpubInfoClass == NULL) {
            LOGI("nativeOpenDocument | FindClass com/chaoxing/epub/nativeapi/EpubInfo  falied");
            return NULL;
        }

        jmethodID methodID_Init = env->GetMethodID(jEpubInfoClass, "<init>", "()V");

        jobject epubInfo = env->NewObject(jEpubInfoClass, methodID_Init);

        jfieldID fieldTitle = env->GetFieldID(jEpubInfoClass, "title", "Ljava/lang/String;");
        jfieldID fieldCreator = env->GetFieldID(jEpubInfoClass, "creator", "Ljava/lang/String;");
        jfieldID fieldDescription = env->GetFieldID(jEpubInfoClass, "description",
                                                    "Ljava/lang/String;");
        jfieldID fieldLanguage = env->GetFieldID(jEpubInfoClass, "language", "Ljava/lang/String;");
        jfieldID fieldContributor = env->GetFieldID(jEpubInfoClass, "contributor",
                                                    "Ljava/lang/String;");
        jfieldID fieldPublisher = env->GetFieldID(jEpubInfoClass, "publisher",
                                                  "Ljava/lang/String;");
        jfieldID fieldSubject = env->GetFieldID(jEpubInfoClass, "subject", "Ljava/lang/String;");
        jfieldID fieldIdentifier = env->GetFieldID(jEpubInfoClass, "identifier",
                                                   "Ljava/lang/String;");

        //env->SetObjectField(epubInfo, fieldAuthor, )

        env->DeleteLocalRef(jEpubInfoClass);
        return epubInfo;
    } else {
        env->ReleaseStringUTFChars(bookPath, c_bookPath);
        return NULL;
    }


}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeGetCatalog
 * Signature: ()[Lcom/chaoxing/epub/nativeapi/Catalog;
 */
JNIEXPORT jobjectArray JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeGetCatalog
        (JNIEnv *env, jobject obj) {
    jobjectArray objArray = NULL;
    SZEbookReader *bookReader = GetBookReader(env, obj);
    if (g_status == 1 || g_status == 3) {
        if (bookReader->bookreader.m_book->m_bookChapter->bHaveChapter()) {

            jclass catalogClass = env->FindClass("com/chaoxing/epub/nativeapi/Catalog");
            if (catalogClass == NULL) {
                LOGI("nativeGetCatalog | FindClass  com/chaoxing/epub/nativeapi/Catalog  falied");
                return objArray;
            }
            jmethodID methodID = env->GetMethodID(catalogClass, "<init>", "()V");

            jfieldID fieldTitle = env->GetFieldID(catalogClass, "title", "Ljava/lang/String;");
            jfieldID fieldLevel = env->GetFieldID(catalogClass, "level", "I");
            jfieldID fieldIndex = env->GetFieldID(catalogClass, "index", "I");
            jfieldID fieldStartPage = env->GetFieldID(catalogClass, "startPage", "I");
            jfieldID fieldEndPage = env->GetFieldID(catalogClass, "endPage", "I");

            MIW orderChapter = bookReader->bookreader.m_book->m_bookChapter->GetMapOrderChapter();
            map<int, pageNumberRange> pageRange = bookReader->GetChapterPageNumberRange();

            jsize len = orderChapter.size();
            objArray = env->NewObjectArray(len, catalogClass, 0);

            MIW::iterator posChapter;
            map<int, pageNumberRange>::iterator posRange;
            int i = 0;
            for (posChapter = orderChapter.begin(), posRange = pageRange.begin();
                 posChapter != orderChapter.end();
                 posChapter++, posRange++) {

                jobject objCatalog = env->NewObject(catalogClass, methodID);

                jstring jchapter = w2js(env, posChapter->second.chapter);
                env->SetObjectField(objCatalog, fieldTitle, jchapter);
                env->SetIntField(objCatalog, fieldLevel, posChapter->second.level);
                env->SetIntField(objCatalog, fieldIndex, posChapter->first);
                env->SetIntField(objCatalog, fieldStartPage, posRange->second.startNumber);
                env->SetIntField(objCatalog, fieldEndPage, posRange->second.endNumber);

                //add to objcet array
                env->SetObjectArrayElement(objArray, i, objCatalog);

                env->DeleteLocalRef(jchapter);
                env->DeleteLocalRef(objCatalog);
                env->DeleteLocalRef(catalogClass);
                i++;
            }
        }
    }
    return objArray;
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeGetFileCount
 * Signature: ()I
 */

JNIEXPORT jint JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeGetFileCount
        (JNIEnv *env, jobject obj) {
    SZEbookReader *bookReader = GetBookReader(env, obj);
    return bookReader->GetFileCount();
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeGetPageCountByFile
 * Signature: (I)I
 */

JNIEXPORT jint JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeGetPageCountByFile
        (JNIEnv *env, jobject obj, jint fileid) {
    SZEbookReader *bookReader = GetBookReader(env, obj);

    return bookReader->GetFilePageCount(fileid);
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeLocateCatalog
 * Signature: (I)[I
 */

JNIEXPORT jintArray JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeLocateCatalog
        (JNIEnv *, jobject, jint) {
    jintArray a;
    return a;
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeDrawPage
 * Signature: (IILandroid/graphics/Bitmap;)I
 */

JNIEXPORT jint JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeDrawPage
        (JNIEnv *env, jobject obj, jint fileid, jint smallPageNumber, jobject bitmapObj) {
    SZEbookReader *bookReader = GetBookReader(env, obj);

    long lSize = 0;
    SZImageBit24 bmp;
    t_PageOutData pageOutData;

    myBYTE * bitmapData/*jbyte *bitmapData*/ = (myBYTE *) bookReader->GetPageByFileSmallPageNumber(fileid, smallPageNumber,
                                                                           bmp, lSize, pageOutData);

    if (bitmapData == NULL) {
        return 1;
    }

    char szpath[256];
    sprintf(szpath,"/sdcard/%d_%d.bmp",fileid,smallPageNumber);
    bmp.writeBMPFile(szpath);
    AndroidBitmapInfo info;
    int ret;
    memset(&info, 0, sizeof(info));
    if ((ret = AndroidBitmap_getInfo(env, bitmapObj, &info)) < 0) {
        LOGI("nativeDrawPage | AndroidBitmap_getInfo error :%d", ret);
        return 2;
    }

    if (info.width <= 0 || info.height <= 0) {
        LOGI("nativeDrawPage | bitmap width or height is zero");
        return 3;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGI("nativeDrawPage | bitmap format is not  RGBA_8888");
        return 4;
    }

    void *pixels = NULL;
    ret = AndroidBitmap_lockPixels(env, bitmapObj, &pixels);
    if (pixels == NULL || ret < 0) {
        LOGI("nativeDrawPage | bitmap lock pixels error : %d", ret);
        return 5;
    }

    for (int y = 0; y < info.height; y++) {
        for (int x = 0; x < info.width; x++) {
            void *pixel = ((uint32_t *)pixels) + y * info.width + x;
            myRGBQUAD color = bmp.GetOriPixel(x,bmp.m_iHeight-y-1);
            *((uint32_t *)pixel) = MAKE_RGBA(color.rgbRed, color.rgbGreen,color.rgbBlue, 255);
        }
    }

    //memcpy(pixels, bitmapData, lSize);

    AndroidBitmap_unlockPixels(env, bitmapObj);
    return 0;
}

/*
 * Class:     com_chaoxing_epub_nativeapi_EpubDocument
 * Method:    nativeCloseDocument
 * Signature: ()V
 */

JNIEXPORT void JNICALL Java_com_chaoxing_epub_nativeapi_EpubDocument_nativeCloseDocument
        (JNIEnv *env, jobject obj) {
    LOGI("nativeCloseDocument");
    SZEbookReader *bookReader = GetBookReader(env, obj);
    bookReader->CloseEbook();

    ReleaseBookReader();

}
}
