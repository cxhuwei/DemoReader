package com.chaoxing.epub.nativeapi;

public class EpubDocument {

    static {
        System.loadLibrary("epub");
    }

    public static native String sayHello();

    protected static native EpubDocument openDocumentWithPath(String path);

}
