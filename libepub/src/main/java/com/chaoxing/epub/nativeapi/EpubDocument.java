package com.chaoxing.epub.nativeapi;

public class EpubDocument {

    static {
        System.loadLibrary("epub");
    }

    public static native String sayHello();

    public static native EpubDocument openDocument(String path);

}
