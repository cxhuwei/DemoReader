package com.chaoxing.epub.nativeapi;

/**
 * Created by HUWEI on 2018/5/2.
 */
public class EpubInfo {

    public String title;
    public String creator;
    public String description;
    public String language;
    public String contributor;
    public String publisher;
    public String subject;
    public String identifier;

    EpubInfo() {
    }

    @Override
    public String toString() {
        return "EpubInfo{" +
                "title='" + title + '\'' +
                ", creator='" + creator + '\'' +
                ", description='" + description + '\'' +
                ", language='" + language + '\'' +
                ", contributor='" + contributor + '\'' +
                ", publisher='" + publisher + '\'' +
                ", subject='" + subject + '\'' +
                ", identifier='" + identifier + '\'' +
                '}';
    }

}
