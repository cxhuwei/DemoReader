package com.chaoxing.epub.nativeapi;

public class EpubDocument {

    static {
        System.loadLibrary("epub");
    }

    public static final String META_FORMAT = "format";
    public static final String META_INFO_AUTHOR = "info:Author";
    public static final String META_INFO_TITLE = "info:Title";

    public static native String sayHello();

    public static native EpubDocument openDocument(String path);

    public static native String getMetaData(String key);

    public static native Chapter[] loadChapter();


}
