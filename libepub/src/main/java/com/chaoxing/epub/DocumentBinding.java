package com.chaoxing.epub;

import java.util.List;
import java.util.TreeMap;

/**
 * Created by HUWEI on 2018/5/14.
 */
public class DocumentBinding {

    private String path;
    private String md5;
    private float density;
    private int width;
    private int height;
    private int fileCount;
    private TreeMap<Integer, List<Resource<EpubPage>>> mResourcePages = new TreeMap<>();

    public DocumentBinding() {
    }


    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public String getMd5() {
        return md5;
    }

    public void setMd5(String md5) {
        this.md5 = md5;
    }

    public float getDensity() {
        return density;
    }

    public void setDensity(float density) {
        this.density = density;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public int getFileCount() {
        return fileCount;
    }

    public void setFileCount(int fileCount) {
        this.fileCount = fileCount;
    }

    public List<Resource<EpubPage>> getResourcePagesByFileId(int fileId) {
        return mResourcePages.get(fileId);
    }

    public void addResourcePages(int fileId, List<Resource<EpubPage>> resourcePages) {
        mResourcePages.put(fileId, resourcePages);
    }

    public void updatePage(Resource<EpubPage> newResourcePage) {
        List<Resource<EpubPage>> pageList = mResourcePages.get(newResourcePage.getData().getFileId());
        if (pageList != null) {
            for (int i = 0; i < pageList.size(); i++) {
                Resource<EpubPage> oldResourcePage = pageList.get(i);
                if (oldResourcePage.getData().getPageNumber() == newResourcePage.getData().getPageNumber()) {
                    pageList.remove(i);
                    pageList.add(i, newResourcePage);
                    break;
                }
            }
        }
    }

}
