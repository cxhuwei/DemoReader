package com.chaoxing.epub;

public class EpubDocument {

    static {
        System.loadLibrary("native-epub");
    }

    public static native String sayHello();

    protected static native EpubDocument openDocumentWithPath(String path);

}
