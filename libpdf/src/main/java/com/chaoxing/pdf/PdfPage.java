package com.chaoxing.pdf;

import com.artifex.mupdf.fitz.Link;
import com.artifex.mupdf.fitz.Rect;

/**
 * Created by HUWEI on 2018/4/18.
 */
public class PdfPage {

    private int number;
    private String pageFile;
    private Link[] links;
    private Rect[] hits;

    public PdfPage() {
    }

    public PdfPage(int number) {
        this.number = number;
    }

    public int getNumber() {
        return number;
    }

    public void setNumber(int number) {
        this.number = number;
    }

    public String getPageFile() {
        return pageFile;
    }

    public void setPageFile(String pageFile) {
        this.pageFile = pageFile;
    }

    public Link[] getLinks() {
        return links;
    }

    public void setLinks(Link[] links) {
        this.links = links;
    }

    public Rect[] getHits() {
        return hits;
    }

    public void setHits(Rect[] hits) {
        this.hits = hits;
    }

}
