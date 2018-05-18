#include "StdAfx.h"
#include "Book.h"
#include "MAlgorithm.h"


#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "SZEbookReader"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG ,__VA_ARGS__)
#endif

CBook::CBook(void *point) {
    m_unzFile = point;
    m_curFile = NULL;
    m_curFile = new CZLFile((unzFile *) point);
    m_curCss = NULL;
    m_curCss = new CZLFile((unzFile *) point);
//	m_otherFile = NULL;
//	m_otherFile = new CZLFile((unzFile*)point);
    m_bookInfo = new CBookInfo();
    m_bookChapter = new CBookChapter();
    m_bookSpine = new CBookSpine();
}

CBook::~CBook(void) {
    if (m_curCss) {
        delete m_curCss;
        m_curCss = NULL;
    }
    if (m_curFile) {
        delete m_curFile;
        m_curFile = NULL;
    }
// 	if (m_otherFile)
// 	{
// 		delete m_otherFile;
// 		m_otherFile = NULL;
// 	}
    if (m_bookSpine) {
        delete m_bookSpine;
        m_bookSpine = NULL;
    }
    if (m_bookInfo) {
        delete m_bookInfo;
        m_bookInfo = NULL;
    }
    if (m_bookChapter) {
        delete m_bookChapter;
        m_bookChapter = NULL;
    }
    //ClearVContent();
}

void CBook::PutCoverFile(string cover) {
    m_coverFileName = cover;
}

std::string CBook::GetCoverFile() {
    if (m_coverFileName == "") {
        m_coverFileName = "ops/images/cover.jpg";
    }
    return m_coverFileName;
}

void CBook::ProcessHtmlCSS(bool isHaveCSS) {
    LOGI("CBook::ProcessHtmlCSS begin");
    bool isContent(false);//\u5F53\u524Dhtml\u662F\u5426\u6709\u6B63\u6587
    bool isP_start(false);
    bool isP_end(false);
    bool isA(false);
    bool isOnlyImage(true);//��ǰhtml�Ƿ�ֻ��ͼƬ����Ч��Ϣ
    bool isOneH(false);
    ClearVContent();
    unsigned int i, j;
    int k;
    VtHTMLNode v_queueBuffer;
    vector<Content *> tempBuffer;
    LOGI("CBook::ProcessHtmlCSS 1 | m_curFile->m_vHtmlNode.size:%d", m_curFile->m_vHtmlNode.size());
    for (i = 0; i < m_curFile->m_vHtmlNode.size(); i++) {
        HTMLNode htmlnode = m_curFile->m_vHtmlNode[i];
        if (htmlnode.nodeType == NODE_START_TAG) {
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_START_TAG | i:%d", i);
            if (htmlnode.tagType == TAG_IMG) {
                goto IMG;
            }
            if (htmlnode.tagType == TAG_P || htmlnode.tagType == TAG_H1 ||
                htmlnode.tagType == TAG_H2 || htmlnode.tagType == TAG_H3
                || htmlnode.tagType == TAG_H4 || htmlnode.tagType == TAG_H5 ||
                htmlnode.tagType == TAG_H6) {
                if (htmlnode.tagType != TAG_P) {
                    isOneH = true;
                }
                isP_start = true;
            }
            v_queueBuffer.push_back(htmlnode);//\u653E\u5165\u7F13\u5B58\u961F\u5217
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_START_TAG 2| i:%d", i);
        } else if (htmlnode.nodeType == NODE_CONTENT) {
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_CONTENT | i:%d", i);
            if (/*isOneH == false &&*/ SpcaceNumber(htmlnode.text) == wcslen(htmlnode.text)) {
                continue;
            }
            Content *wenben = new Content();
            if (wenben == NULL) {
                return;
            }
            if (isP_start) {
                wenben->isP_start = true;
                isP_start = false;
            }
            //\u4ECE\u7B2C\u4E00\u4E2A\u6807\u7B7E\u5F00\u59CB\u67E5\u9605css\u6837\u5F0F?\u6216\u8005\u4ECE\u6700\u540E\u4E00\u4E2A\u5F00\u59CB?
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_CONTENT 2| i:%d", i);
            for (j = 0; j < v_queueBuffer.size(); j++) {
                HTMLNode node = v_queueBuffer[j];
                if (node.tagType == TAG_A) {
                    wenben->contentType |= CT_Link;
                    wenben->textStyle.UnderlineIsVaild = true;
                    wenben->textStyle.isUnderline = true;
                    isA = true;
                }
                if (node.tagType == TAG_SUP) {
                    wenben->contentType |= CT_SUP;
                }

                m_curFile->GetFrontStyleFormTag(node.tagType, wenben);
                //2step ���ݱ�ǩ�Ҷ�Ӧcss
                if (isHaveCSS) {
                    map<string, StyleSheet>::iterator pos = m_curCss->m_MCSS.find(
                            m_ToLower(node.tagName));
                    if (pos != m_curCss->m_MCSS.end()) {
                        int bold = wenben->textStyle.Bold;
                        StyleSheet ss = pos->second;
                        wenben->textStyle = ss;
                        if (bold == 1) {
                            wenben->textStyle.Bold = 1;
                        }
                        wenben->isHaveStyle = true;
                    }
                }

                LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_CONTENT 3| i:%d", i);
                int attrnum = node.propCount;
                for (k = 0; k < attrnum; k++) {
                    //3step \u6839\u636E\u6807\u7B7E\u5185\u90E8\u5C5E\u6027 class/id\u5BF9\u5E94css
                    if (isHaveCSS) {
                        if (strcmp(node.nodeProps[k].szName, "class") == 0 ||
                            strcmp(node.nodeProps[k].szName, "id") == 0) {
                            string strValue = m_ToLower(node.nodeProps[k].szValue);
                            if (strcmp(node.nodeProps[k].szName, "class") == 0) {
                                strValue.insert(0, ".");
                            }
                            if (strcmp(node.nodeProps[k].szName, "id") == 0) {
                                strValue.insert(0, "#");
                            }
                            map<string, StyleSheet>::iterator pos = m_curCss->m_MCSS.find(strValue);
                            if (pos != m_curCss->m_MCSS.end()) {
                                int bold = wenben->textStyle.Bold;
                                wenben->textStyle = pos->second;
                                wenben->isHaveStyle = true;
                                if (bold == 1) {
                                    wenben->textStyle.Bold = 1;
                                }
                            }
                        }
                    }

                    //4step \u6807\u7B7E\u5185\u7F6Estyle
                    if (strcmp(node.nodeProps[k].szName, "style") == 0) {
                        CStyleSheetParser tempcss;
                        tempcss.PutData((myBYTE *) (node.nodeProps[k].szValue),
                                        strlen(node.nodeProps[k].szValue));
                        tempcss.Parser();
                        if (tempcss.m_MCSS.size() != 0) {
                            map<string, StyleSheet>::iterator pos = tempcss.m_MCSS.begin();
                            wenben->textStyle = pos->second;
                            wenben->isHaveStyle = true;
                        }

                    }
                    //5 step \u8D85\u94FE\u63A5
                    if (isA) {
                        if (strcmp(node.nodeProps[k].szName, "href") == 0) {
                            if (node.nodeProps[k].szValue[0] == '#') {
                                wenben->filepath.append(m_curFile->GetCurFilePath());
                                wenben->filepath.append(node.nodeProps[k].szValue);
                            } else {
                                string curdir = m_curFile->GetCurDir();
                                wenben->filepath = curdir;
                                wenben->filepath.append("/");
                                MergerDir(wenben->filepath, node.nodeProps[k].szValue);
                            }

                            size_t pos = wenben->filepath.find("//");
                            if (pos != string::npos) {
                                wenben->filepath.replace(pos, 2, "/");
                            }


                        }
                    }
                }
                LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_CONTENT 4| i:%d", i);
            }
            //\u5904\u7406\u4E32\u524D\u7A7A\u683C
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_CONTENT 5| i:%d", i);
            int befor, after;
            myWcharT kongge = 0x0020;
            myWcharT kongge2 = 0x3000;
            myWcharT huanhang = L'\n';
            myWcharT *temptext = m_curFile->m_vHtmlNode[i].text;
            int textLegth = myWcslen(m_curFile->m_vHtmlNode[i].text);
            if (SpcaceNumber(htmlnode.text) == wcslen(htmlnode.text)) {
                befor = 0;
                after = textLegth - 1;
            } else {
                for (befor = 0; befor < textLegth; befor++) {
                    if (temptext[befor] != kongge && temptext[befor] != kongge2 &&
                        temptext[befor] != huanhang) {
                        break;
                    }
                }
                for (after = textLegth - 1; after >= 0; after--) {
                    if (temptext[after] != kongge && temptext[after] != kongge2 &&
                        temptext[after] != huanhang) {
                        break;
                    }
                }
                /*int beforLength,afterLength;*/
                if (befor > 0) {
                    wenben->before = new myWcharT[befor + 1];
                    if (wenben->before == NULL) {
                        delete wenben;
                        wenben = NULL;
                        return;
                    }
                    memset(wenben->before, 0, sizeof(myWcharT) * (befor + 1));
                    memcpy(wenben->before, temptext, befor * sizeof(myWcharT));
                } else
                    wenben->before = NULL;
                if (after < textLegth - 1 && after >= befor) {
                    wenben->after = new myWcharT[textLegth - after];
                    if (wenben->after == NULL) {
                        if (wenben->before) {
                            delete[] wenben->before;
                            wenben->before = NULL;
                        }
                        delete wenben;
                        wenben = NULL;
                        return;
                    }
                    memset(wenben->after, 0, sizeof(myWcharT) * (textLegth - after));
                    if (after < 0) {
                        memcpy(wenben->after, temptext, textLegth * sizeof(myWcharT));
                    } else
                        memcpy(wenben->after, temptext + after,
                               (textLegth - after - 1) * sizeof(myWcharT));

                } else
                    wenben->after = NULL;
            }
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_CONTENT 6| i:%d", i);
            if (after >= befor) {
                wenben->text = new myWcharT[after - befor + 2];
                if (wenben->text == NULL) {
                    if (wenben->before) {
                        delete[] wenben->before;
                        wenben->before = NULL;
                    }
                    if (wenben->after) {
                        delete[] wenben->after;
                        wenben->after = NULL;
                    }
                    delete wenben;
                    return;
                }
                memset(wenben->text, 0, sizeof(myWcharT) * (after - befor + 2));
                memcpy(wenben->text, temptext + befor, (after - befor + 1) * sizeof(myWcharT));
                isContent = true;
                isOnlyImage = false;
            } else
                wenben->text = NULL;
            if (isOneH)
                tempBuffer.push_back(wenben);
            else
                m_vContent.push_back(wenben);
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_CONTENT 7| i:%d", i);
        } else if (htmlnode.nodeType == NODE_CLOSE_TAG) {
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_CLOSE_TAG | i:%d", i);
            if (!v_queueBuffer.empty()) {

                HTMLNode pos = v_queueBuffer[v_queueBuffer.size() - 1];
                if (pos.tagType == TAG_P || pos.tagType == TAG_H1 || pos.tagType == TAG_H2 ||
                    pos.tagType == TAG_H3 || pos.tagType == TAG_H4
                    || pos.tagType == TAG_H5 || pos.tagType == TAG_H6 || pos.tagType == TAG_LI ||
                    pos.tagType == TAG_DIV) {
                    //\u5F39\u51FA\u6700\u540E\u4E00\u4E2A\u4FEE\u6539isp\u7136\u540E\u518D\u653E\u8FDB\u53BB
                    if (m_vContent.size() > 0 &&
                        !isOneH/*&& (pos.tagType == TAG_P || pos.tagType == TAG_DIV)*/) {
                        Content *modify = m_vContent[m_vContent.size() - 1];
                        modify->isP_end = true;
                    }

                    if (pos.tagType != TAG_P && pos.tagType != TAG_LI && pos.tagType != TAG_DIV) {
                        isOneH = false;
                        int h;
                        int textlength(0);
                        Content *hebingContent = new Content();
                        if (hebingContent == NULL) {
                            continue;
                        }
                        hebingContent->isP_start = true;
                        hebingContent->isP_end = true;
                        for (h = 0; h < tempBuffer.size(); h++) {
                            Content *temp = tempBuffer[h];
                            if (temp->text) {
                                textlength += myWcslen(temp->text);
                            }

                        }
                        hebingContent->text = new myWcharT[textlength + 1];
                        if (hebingContent->text == NULL) {
                            continue;
                        }
                        hebingContent->after = hebingContent->before = NULL;
                        memset(hebingContent->text, 0, (textlength + 1) * sizeof(myWcharT));
                        int pos(0);
                        for (h = 0; h < tempBuffer.size(); h++) {
                            Content *temp = tempBuffer[h];
                            if (h == 0) {
                                int l;
                                hebingContent->textStyle = temp->textStyle;
                                //hebingContent->textStyle.textAlign=CENTER;
                                hebingContent->isHaveStyle = true;
                                hebingContent->textStyle.Bold = true;
                                hebingContent->contentType = temp->contentType;
                                hebingContent->titleType = temp->titleType;
                                if (temp->before) {

                                    l = myWcslen(temp->before);
                                    hebingContent->before = new myWcharT[l + 1];
                                    memset(hebingContent->before, 0, (l + 1) * sizeof(myWcharT));
                                    if (hebingContent->before != NULL) {
                                        memcpy(hebingContent->before, temp->before,
                                               l * sizeof(myWcharT));
                                    }
                                } else
                                    hebingContent->before = NULL;
                                if (temp->after) {
                                    l = myWcslen(temp->after);
                                    hebingContent->after = new myWcharT[l + 1];
                                    memset(hebingContent->after, 0, (l + 1) * sizeof(myWcharT));
                                    if (hebingContent->after != NULL) {
                                        memcpy(hebingContent->after, temp->after,
                                               l * sizeof(myWcharT));
                                    }
                                } else
                                    hebingContent->after = NULL;
                            }
                            if (temp->text) {
                                int length = myWcslen(temp->text);
                                memcpy(hebingContent->text + pos, temp->text,
                                       length * sizeof(myWcharT));
                                pos += length;
                            }

                            //clear
                            if (temp->before) {
                                delete[] temp->before;
                                temp->before = NULL;
                            }
                            if (temp->after) {
                                delete[] temp->after;
                                temp->after = NULL;
                            }
                            if (temp->text) {
                                delete[] temp->text;
                                temp->text = NULL;
                            }
                            delete temp;
                            temp = NULL;
                        }
                        m_vContent.push_back(hebingContent);
                        tempBuffer.clear();
                    }
                }
                v_queueBuffer.pop_back();
            }
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_CLOSE_TAG 2| i:%d", i);
        } else if (htmlnode.nodeType == NODE_FULL_TAG) {//\u5904\u7406img
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_FULL_TAG | i:%d", i);
            if (htmlnode.tagType == TAG_BR && m_vContent.size() > 0) {
                if (m_vContent.size() > 0) {
                    Content *modify = m_vContent[m_vContent.size() - 1];
                    modify->isP_end = true;
                }
            }
            if (htmlnode.tagType == TAG_IMG) {
                IMG:
                Content *image = new Content();
                if (image == NULL) {
                    return;
                }
                image->contentType = CT_Image;
                string temp;
                string imgfile;
                for (int i = 0; i < htmlnode.propCount; i++) {
                    if (strcmp(htmlnode.nodeProps[i].szName, "src") == 0 ||
                        strcmp(htmlnode.nodeProps[i].szName, "xlink:href") == 0) {
                        temp.assign(htmlnode.nodeProps[i].szValue);
                    }
                    if (strcmp(htmlnode.nodeProps[i].szName, "align") == 0) {
                        switch (htmlnode.nodeProps[i].szValue[0]) {
                            case 'c':
                                image->textStyle.textAlign = CENTER;
                                break;
                            case 'l':
                                image->textStyle.textAlign = LEFT;
                                break;
                            case 'r':
                                image->textStyle.textAlign = RIGHT;
                                break;
                            default:
                                image->textStyle.textAlign = CENTER;
                        }
                    }
                }
                if (!temp.empty()) {
                    string curdir = m_curFile->GetCurDir();
                    if (!curdir.empty()) {
                        if (temp.at(0) != '/') {
                            temp.insert(0, "/");
                        }
                    }

                    //"../"���Ƶ�·��

                    string::size_type postion = temp.find("../");
                    while (postion != temp.npos) {
                        string::size_type p = curdir.rfind("/");
                        if (p != curdir.npos) {
                            curdir.erase(p, curdir.npos);
                        }
                        temp.erase(postion, 3);
                        postion = temp.find("../");
                    }

                    imgfile = curdir + temp;
                }
                image->filepath = imgfile;
                m_vContent.push_back(image);
                isContent = true;
            }
            LOGI("CBook::ProcessHtmlCSS htmlnode.nodeType == NODE_FULL_TAG 2| i:%d", i);
        }
    }
    LOGI("CBook::ProcessHtmlCSS 2");
    //����ֻ��ͼƬ��Ϣҳ��
    if (isOnlyImage) {
        vector<Content *>::iterator pos;
        for (pos = m_vContent.begin(); pos != m_vContent.end();/*pos++*/) {
            Content *p = *pos;
            if (p->contentType == CT_Image) {
                p->contentType = /*CT_Image*/CT_ImagePage;
                pos++;
            } else {
                if (p->before) {
                    delete[] p->before;
                    p->before = NULL;
                }
                if (p->text) {
                    delete[] p->text;
                    p->text = NULL;
                }
                if (p->after) {
                    delete[] p->after;
                    p->after = NULL;
                }
                delete p;
                p = NULL;
                pos = m_vContent.erase(pos);
            }
            if (pos == m_vContent.end()) {
                break;
            }
        }
    }
    if (!isContent) {
        ClearVContent();
    }
    v_queueBuffer.clear();
    LOGI("CBook::ProcessHtmlCSS 3");
    m_curFile->ClearHtmlData();//\u91CA\u653Em_vhtmlnode\u5185\u5B58
    LOGI("CBook::ProcessHtmlCSS 4");
}

void CBook::ClearVContent(bool isTxt) {
    if (isTxt) {
        for (int i = 0; i < m_vContent.size(); i++) {
            Content *con = m_vContent[i];
            if (con->after) {
                delete[] con->after;
                con->after = NULL;
            }
            if (con->before) {
                delete[] con->before;
                con->before = NULL;
            }
            if (con->txt_SrcPtext) {
                delete[] con->txt_SrcPtext;
                con->txt_SrcPtext = NULL;
            }
            delete con;
            con = NULL;
        }
    }

    m_vContent.clear();
}

map<string, int> CBook::GetCurFileAnchorContentNumber(void) {
    return m_curFile->GetAnchorContentNumber();
}

int CBook::parser(string &filepath) {
    LOGI("CBook::parser | begin");
    string ext(m_curFile->GetExtension());

    if (ext == "html" || ext == "htm" || ext == "xhtml" || ext == "xml") {
        try {
            if (0 != m_curFile->Parser()) {
                LOGI("CBook::parser | end return -3");
                return -3;
            }
            LOGI("CBook::parser | 1");
        }
        catch (...) {
            LOGI("CBook::parser | catch exception");
        }
        m_FIleAnchorContentNumber.insert(
                pair<string, MSI>(filepath, m_curFile->GetAnchorContentNumber()));
        LOGI("CBook::parser | 2");
        if (m_curFile->IsHaveCSS()) {
            LOGI("CBook::parser | 3");
            vector<string> cssFiles = m_curFile->ReturnCSSFilepath();
            LOGI("CBook::parser | 4");
            for (int i = 0; i < cssFiles.size(); i++) {
                LOGI("CBook::parser | m_curCss->OpenFile | i:%d", i);
                if (m_curCss->OpenFile(cssFiles[i].c_str(), true, false) >=
                    0) {//\u89E3\u6790\u8FC7css\u4E0D\u9700\u8981\u518D\u6B21\u89E3\u6790
                    LOGI("CBook::parser | m_curCss->OpenFile success will parsercss| i:%d", i);
                    m_curCss->ParserCSS();
                    LOGI("CBook::parser | m_curCss->OpenFile success finish parsercss| i:%d", i);
                }
            }


        }
        if (!m_curFile->LastOpen()) {
            LOGI("CBook::parser |  will ProcessHtmlCSS ");
            ProcessHtmlCSS(m_curFile->IsHaveCSS());
            LOGI("CBook::parser |  finish ProcessHtmlCSS ");
        }

    } else if (ext == "css") {

    }
    LOGI("CBook::parser | end");
    return 0;
}

void CBook::CopyMapAnchorContent(CBook *book) {
    this->m_FIleAnchorContentNumber = book->m_FIleAnchorContentNumber;
}

Record CBook::GetChapterInforFromChapterOrder(int order) {
    Record chapterinfo;
    chapterinfo.contentNumber = 0;
    chapterinfo.offer = 0;
    string filepath;
    if (m_unzFile == NULL) {//txt�ߵ�����
        myWcharT *chapter = NULL;
        m_bookChapter->GetFileByOrder(order, filepath);
        chapterinfo.file = filepath;
        if (m_bookChapter->GetChapterByOrder(order, chapter)) {
            //unicode to ansi
            CStringConver sc;
            int len = myWcslen(chapter);
            char *anChapter = new char[len * 2 + 1];
            memset(anChapter, 0, len * 2 + 1);
            sc.UnicodeBigToGbk(chapter, len * sizeof(myWcharT), anChapter, len * 2);
            if (m_FIleAnchorContentNumber.size() > 0) {
                map<string, MSI>::iterator pos = m_FIleAnchorContentNumber.begin();
                if (pos != m_FIleAnchorContentNumber.end()) {
                    MSI tempmap = pos->second;
                    MSI::iterator pos2 = tempmap.find(anChapter);
                    if (pos2 != tempmap.end()) {
                        chapterinfo.contentNumber = pos2->second;
                    }
                }
            }
            delete[] anChapter;
            anChapter = NULL;
        }

    } else {
        if (m_bookChapter->GetFileByOrder(order, filepath)) {
            chapterinfo = GetChapterInforFromFilePath(filepath);
        }
    }

    return chapterinfo;

}

void CBook::PutBookName(string &bookname) {
    this->m_bookname.assign(bookname);
}

std::string CBook::GetBookName() {
    return m_bookname;
}

Record CBook::GetChapterInforFromFilePath(string filepath) {
    Record chapterinfo;
    chapterinfo.contentNumber = 0;
    chapterinfo.offer = 0;
    char anchorName[SHORTNAME_LENGTH];
    int length(0);
    const char *szfilepath = filepath.c_str();
    const char *anchor = strstr(szfilepath, "#");
    if (anchor != NULL) {
        length = anchor - szfilepath;
        memset(anchorName, 0, SHORTNAME_LENGTH);//\u9632\u6B62\u518D\u8D4B\u503C
        memcpy(anchorName, anchor + 1, strlen(szfilepath) - length - 1);
    }
    if (length > 0) {
        char *temp = new char[length + 1];
        if (temp) {
            memset(temp, 0, length + 1);
            memcpy(temp, szfilepath, length);
        }
        chapterinfo.file.assign(temp);
        if (temp) {
            delete[] temp;
        }
    } else
        chapterinfo.file.assign(szfilepath);

    //���ê�� ��Ҫ����һ��html
    if (m_unzFile) {
        OpenFileByFilePath(chapterinfo.file);
    }


    if (m_FIleAnchorContentNumber.size() > 0) {
        map<string, MSI>::iterator pos = m_FIleAnchorContentNumber.find(chapterinfo.file);
        if (pos != m_FIleAnchorContentNumber.end()) {
            MSI tempmap = pos->second;
            MSI::iterator pos2 = tempmap.find(anchorName);
            if (pos2 != tempmap.end()) {
                chapterinfo.contentNumber = pos2->second;
            }
        }
    }
    return chapterinfo;
}

int CBook::OpenFileByFilePath(string filepath) {
    CZLFile *otherfile = new CZLFile((unzFile *) m_unzFile);
    if (otherfile == NULL) {
        return -1;
    }
    int er = otherfile->OpenFile(filepath.c_str());
    if (er >= 0) {
        if (0 != otherfile->Parser()) {
            er = -2;
        } else
            m_FIleAnchorContentNumber.insert(
                    pair<string, MSI>(filepath, otherfile->GetAnchorContentNumber()));
    }
    delete otherfile;
    otherfile = NULL;
    return er;

}

