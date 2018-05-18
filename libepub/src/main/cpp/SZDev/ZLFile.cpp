#include "StdAfx.h"
#include "ZLFile.h"
#include "zlib.h"
#include "MAlgorithm.h"
#include "ThreadPacket.h"
#include "StringConver.h"


#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "SZEbookReader"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG ,__VA_ARGS__)
#endif

THREAD_MUTEX gg_lock;
int first(0);
extern "C"
{
int do_extract_currentfile(unzFile uf, const int *popt_extract_without_path, int *popt_overwrite,
                           char *root);
int ReadOneFileToMem(unzFile uf, char *filename, myBYTE **buffer, int *length);
}

CZLFile::CZLFile(unzFile *unzfile)
        : m_isAnchor(false), m_isLastOpen(false) {
    memset(m_extension, 0, EXTEN_LENGTH);
    memset(m_shortName, 0, SHORTNAME_LENGTH);
    memset(m_anchorName, 0, SHORTNAME_LENGTH);
    m_filePath = NULL;
    m_tmpFilePath = NULL;
    m_unzFile = unzfile;
    m_DataFile = NULL;
    m_DataLength = 0;

    m_tAnchor.contentNumber = 0;
    m_tAnchor.offer = 0;
    m_HtmlTitle = NULL;
    if (first == 0) {
        first++;
        INITIALIZE_MUTEX(&gg_lock);
        printf("init mutex\n");
    }
}

CZLFile::CZLFile()
        : m_isAnchor(false), m_isLastOpen(false), m_isHaveCSS(false) {
    m_curDir = "";
    m_unzFile = NULL;
    memset(m_extension, 0, EXTEN_LENGTH);
    memset(m_shortName, 0, SHORTNAME_LENGTH);
    memset(m_anchorName, 0, SHORTNAME_LENGTH);
    m_filePath = NULL;
    m_tmpFilePath = NULL;
    m_DataFile = NULL;
    m_DataLength = 0;

    m_tAnchor.contentNumber = 0;
    m_tAnchor.offer = 0;
    m_HtmlTitle = NULL;
    INITIALIZE_MUTEX(&gg_lock);

}

CZLFile::~CZLFile(void) {
    if (m_filePath != NULL) {
        delete[] m_filePath;
        m_filePath = NULL;
    }
    if (m_DataFile != NULL) {
        free(m_DataFile);
        m_DataFile = NULL;
    }
    ClearHtmlData();
    ClearCSSData();

}

char *CZLFile::GetExtension(void) {
    if (m_tmpFilePath == NULL) {
        return "";
    }
    int length = strlen(m_tmpFilePath);
    int i;
    for (i = length - 1; i >= 0; i--) {
        if (m_tmpFilePath[i] == '.') {
            break;
        }
    }
    if (i > 0) {
        memset(m_extension, 0, EXTEN_LENGTH);
        memcpy(m_extension, m_tmpFilePath + i + 1, length - i - 1);
    }
    return m_extension;
}

int CZLFile::PutFilePath(const char *filepath) {

    int length;
    const char *anchor = strstr(filepath, "#");
    if (anchor != NULL) {
        m_isAnchor = true;
        length = anchor - filepath;
        memset(m_anchorName, 0, SHORTNAME_LENGTH);//防止再赋值
        memcpy(m_anchorName, anchor + 1, strlen(filepath) - length - 1);
    } else {
        m_isAnchor = false;
        length = strlen(filepath);
    }
    if (m_filePath != NULL) {
        char *temp = new char[length + 1];
        if (temp) {
            memset(temp, 0, length + 1);
            memcpy(temp, filepath, length);
            if (strcmp(temp, m_filePath) == 0) {//上次打开过了就不再打开
                delete[] temp;
                temp = NULL;
                return 1;
            } else {
                delete[] temp;
                temp = NULL;
            }
        }
// 		delete[] m_filePath;
// 		m_filePath = NULL;
    }
    //使用临时文件保存当前路径，如果解压成功才更新m_filePath字段 myf 20141229
// 	m_filePath = new char[length + 1];
// 	if (m_filePath)
// 	{
// 		memcpy(m_filePath,filepath,length);
// 		m_filePath[length] = '\0';
// 		m_tAnchor.file.assign(m_filePath);
// 	}
    if (m_tmpFilePath != NULL) {
        delete[] m_tmpFilePath;
        m_tmpFilePath = NULL;
    }
    m_tmpFilePath = new char[length + 1];
    if (m_tmpFilePath) {
        memcpy(m_tmpFilePath, filepath, length);
        m_tmpFilePath[length] = '\0';
        //m_tAnchor.file.assign(m_tmpFilePath);
    }
    return 0;
}

int CZLFile::OpenFile(const char *file, bool isCleanHtmlData /*= true*/,
                      bool isCleanCssData/*=true*/ ) {
    int i;
    for (i = strlen(file) - 1; i >= 0; i--) {
        if (file[i] == '/') {
            m_curDir.assign(file, i);
            break;
        }
    }
    if (i < 0) {
        m_curDir.assign("");
    }
    m_isLastOpen = false;

    if (0 != PutFilePath(file)) {//说明上次已经打开本文件了

        m_isLastOpen = true;
        return OPENZIPFILE_LASTOPEN;
    }
    m_vCssFiles.clear();
    string ext = GetExtension();
    if ((ext == "html" || ext == "htm" || ext == "xhtml" || ext == "xml") && isCleanHtmlData) {
        ClearHtmlData();
    } else if (ext == "css" && isCleanCssData) {
        ClearCSSData();
    }

    if (m_DataFile != NULL) {
        free(m_DataFile);
        m_DataFile = NULL;
    }
    LOCK_MUTEX(&gg_lock);
    int err = ReadOneFileToMem(*m_unzFile, m_tmpFilePath, &m_DataFile, &m_DataLength);
    if (OPENZIPFILE_OK != err) {
        UNLOCK_MUTEX(&gg_lock);
        if (m_DataFile != NULL) {
            free(m_DataFile);
            m_DataFile = NULL;
        }
        if (err == 1) {
            err = OPENZIPFILE_NOT_EXIST;
        }
        return err;//-1;
    }
    UNLOCK_MUTEX(&gg_lock);
    if (m_DataFile == NULL) {
        return OPENZIPFILE_DATA_ERROR;
    }

    if (m_filePath != NULL) {
        delete[] m_filePath;
        m_filePath = NULL;
    }
    int len = strlen(m_tmpFilePath);
    if (len > 0) {
        m_filePath = new char[len + 1];
        strcpy(m_filePath, m_tmpFilePath);
        m_tAnchor.file.assign(m_filePath);
    }
    return OPENZIPFILE_OK;
}

// 返回文件的数据
myBYTE *CZLFile::GetFileData(void) {
    return m_DataFile;
}

int CZLFile::GetDataLength(void) {
    return m_DataLength;
}

// 解析该文件的html结构
int CZLFile::Parser(void) {
    LOGI("CZLFile::Parser | begin");
    int anchorNumber(0);
    if (!m_isLastOpen) {//上次没打开过，进行解析
        LOGI("CZLFile::Parser | 1");
        CHtmlParser par(this->m_DataFile, this->m_DataLength, &m_vHtmlNode);
        LOGI("CZLFile::Parser | 2");
        m_mAnchorNameContentNumber.clear();
        LOGI("CZLFile::Parser | 3");
        par.PutAnchor(&m_mAnchorNameContentNumber);
        LOGI("CZLFile::Parser | 4");
        if (par.ParserHtml()) {
            LOGI("CZLFile::Parser | 5 1");
            m_HtmlTitle = par.m_title;
            //把html内部css样式放到m_mcss里。
            m_MCSS = par.m_inlineMCSS;
            m_isHaveCSS = par.IsHaveCSS();
            LOGI("CZLFile::Parser | 5 2");
            vector<string> tempcss = par.ReturnCSSFilePath();
            LOGI("CZLFile::Parser | 5 3");
            //m_cssFile = MergePath(tempcss,m_curDir);
            m_cssFile = m_curDir;
            for (int i = 0; i < tempcss.size(); i++) {
                m_cssFile = MergerDir(m_cssFile, tempcss[i]);
                m_vCssFiles.push_back(m_cssFile);
            }
            LOGI("CZLFile::Parser | 5 4");

            //	return 0;
        } else {
            LOGI("CZLFile::Parser | 5 2");
            if (m_DataFile != NULL) {
                free(m_DataFile);
                m_DataFile = NULL;
            }
            return -1;
        }
    }
    if (m_isAnchor) {//需要找到锚点
        string anchorName(m_anchorName);
        map<string, int>::iterator pos = m_mAnchorNameContentNumber.find(anchorName);
        if (pos != m_mAnchorNameContentNumber.end()) {
            m_tAnchor.contentNumber = pos->second;
        }
    }
    if (m_DataFile != NULL) {
        free(m_DataFile);
        m_DataFile = NULL;
    }
    LOGI("CZLFile::Parser | end");
    return 0;
}

void CZLFile::ClearHtmlData() {
    int i, j;
    int length = m_vHtmlNode.size();
    for (i = 0; i < length; i++) {
        /*m_vHtmlNode[i].nodeProps->Clear();*/
        for (j = 0; j < m_vHtmlNode[i].propCount; j++) {
            m_vHtmlNode[i].nodeProps[j].Clear();
        }
        delete[] m_vHtmlNode[i].nodeProps;
        delete[] m_vHtmlNode[i].tagName;
        delete[] m_vHtmlNode[i].text;
    }

    m_vHtmlNode.clear();

    if (m_HtmlTitle) {
        delete[] m_HtmlTitle;
    }
}

bool CZLFile::IsHaveCSS(void) {
    return m_isHaveCSS;
}

vector<string> CZLFile::ReturnCSSFilepath(void) {
    return m_vCssFiles;
}

int CZLFile::ParserCSS(void) {


    CStyleSheetParser cssparser;
    cssparser.PutData(this->m_DataFile, this->m_DataLength);
    cssparser.Parser();
    PutCssIntoFileCss(cssparser);
    return 0;

}

void CZLFile::PutOebpsDir(std::string oebpsdir) {
    m_oebpsDir = oebpsdir;
    if (m_oebpsDir != "") {
        m_oebpsDir.append("/");
    }
}

void CZLFile::GetFrontStyleFormTag(HTMLTagType tagtype, Content *content) {
    switch (tagtype) {
        case TAG_H1:
            content->textStyle.fontSize = 30;
            content->textStyle.Bold = 1;
            content->titleType = 1;
            //content->textStyle.isP = true;
            break;
        case TAG_H2:
            content->textStyle.fontSize = 24;
            content->textStyle.Bold = 1;
            content->titleType = 2;
            //content->textStyle.isP = true;
            break;
        case TAG_H3:
            content->textStyle.fontSize = 20;
            content->textStyle.Bold = 1;
            content->titleType = 3;
            //content->textStyle.isP = true;
            break;
        case TAG_H4:
            content->textStyle.fontSize = 16;
            content->textStyle.Bold = 1;
            content->titleType = 4;
            //content->textStyle.isP = true;
            break;
        case TAG_H5:
            content->textStyle.fontSize = 13;
            content->textStyle.Bold = 1;
            content->titleType = 5;
            //content->textStyle.isP = true;
            break;
        case TAG_H6:
            content->textStyle.fontSize = 10;
            content->textStyle.Bold = 1;
            content->titleType = 6;
            //content->textStyle.isP = true;
            break;
        case TAG_B:
            content->textStyle.Bold = 1;
            //content->textStyle.isP = true;
            break;
// 	case TAG_P:
// 		content->textStyle.isP = true;
        default:
            break;
    }
}

// 把解析css对象中的样式放到zlfile的样式表中。
void CZLFile::PutCssIntoFileCss(CStyleSheetParser &csspar) {
    map<string, StyleSheet>::iterator pos = csspar.m_MCSS.begin();
    for (; pos != csspar.m_MCSS.end(); pos++) {
        string temp = pos->first;
        size_t s = temp.find("/*");
        size_t e = temp.find("*/");
        if (s != string::npos && e != string::npos) {
// 			string::iterator it;
// 			it = temp.begin();
            temp.erase(s, e - s + 2);
        }
        m_TrimString(temp);
        m_MCSS.insert(std::pair<string, StyleSheet>(temp, pos->second));
    }
}

string CZLFile::GetCurDir(void) {
    return m_curDir;
}

string CZLFile::MergePath(string path, string root) {
    string fullpath;
    fullpath = root;
    int pos = path.find("../");
    while (pos >= 0) {
        int last = fullpath.find_last_of("/");
        if (last != string::npos) {
            fullpath.erase(last);
        } else
            fullpath.erase(fullpath.begin(), fullpath.end());
        path.erase(0, pos + 3);
        pos = path.find("../");
    }
    if (!fullpath.empty()) {
        fullpath += "/";
    }
    fullpath += path;
    return fullpath;
}

void CZLFile::ClearCSSData(void) {
    m_MCSS.clear();

}

Record *CZLFile::GetAnchor() {
    return &m_tAnchor;
}

bool CZLFile::LastOpen() {
    return m_isLastOpen;
}

map<string, int> CZLFile::GetAnchorContentNumber() {
    return m_mAnchorNameContentNumber;
}

std::string CZLFile::GetCurFilePath() {
    return m_filePath;
}
