package com.chaoxing.epub;

import android.graphics.Bitmap;

/**
 * Created by HUWEI on 2018/4/27.
 */
public class EpubPage {

    public enum PageType {
        PAGE,
        FILE
    }

    private PageType pageType;
    private int fileId;
    private int pageNumber;
    private Bitmap bitmap;

    public PageType getPageType() {
        return pageType;
    }

    public void setPageType(PageType pageType) {
        this.pageType = pageType;
    }

    public int getFileId() {
        return fileId;
    }

    public void setFileId(int fileId) {
        this.fileId = fileId;
    }

    public int getPageNumber() {
        return pageNumber;
    }

    public void setPageNumber(int pageNumber) {
        this.pageNumber = pageNumber;
    }

    public Bitmap getBitmap() {
        return bitmap;
    }

    public void setBitmap(Bitmap bitmap) {
        this.bitmap = bitmap;
    }

}
