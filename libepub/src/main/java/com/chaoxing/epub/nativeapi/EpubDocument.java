package com.chaoxing.epub.nativeapi;

/**
 * Created by HUWEI on 2018/4/24.
 */
public class EpubDocument {

    static {
        System.loadLibrary("epub");
    }

    public static native String sayHello();

    public static native int openDocument(String path);

    public static native String getTitle();

    public static native String getAuthor();

    public static native int loadFileCount();

    public static native int loadPageCountByFile();

}
