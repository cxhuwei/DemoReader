package com.chaoxing.epub;

/**
 * Created by HUWEI on 2018/5/22.
 */
public class PageKey {

    private int fileId;
    private int pageNumber;

    public PageKey() {
    }

    public PageKey(int fileId, int pageNumber) {
        this.fileId = fileId;
        this.pageNumber = pageNumber;
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

}
