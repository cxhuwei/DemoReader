// SZEbookReader.cpp: implementation of the SZEbookReader class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "SZEbookReader.h"
#include "ReadImageFile.h"
#include "MAlgorithm.h"
#include "tinyxml.h"

#if defined (ANDROID22)

#include "BWImageScalingAlgorithm.h"

#else
#include "BWImageScalingAlgorithm2.h"

#endif


#ifdef USE_CXIMAGE

#include "ximage.h"

#endif

#define BI_RGB        0L
//create the global mutex
//THREAD_MUTEX  g_Mutex;

#ifndef _WIN32
THREAD_MUTEX g_ClacpageMutex;
pthread_cond_t g_cond;
#endif // _WIN32
//////////////////////////////////////////////////////////////////////
#define titleRatio 5
#define fontRank 29
#define vertLineheight 0.5

#ifdef ANDROID22
#define FileType  "myf"

#include <android/log.h>

#define LOG_TAG "SZEbookReader"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG ,__VA_ARGS__)
#else
#define FileType  "txt"
#endif // ANDROID22

bool g_IsDrawing = false;

int threadstartFun(void *pParam) {
    // LOCK_MUTEX(&g_Mutex);
    SZEbookReader *pCaller = (SZEbookReader *) (pParam);
    pCaller->m_isReturn = false;

    pCaller->calcOtherHtml();


    printf("��ȫ�˳������߳�\n");
    pCaller->m_isReturn = true;
    //UNLOCK_MUTEX(&g_Mutex);
    //end the thread
    END_THREAD();
    return 0;
}

unsigned int threadCalcChapterNumberRange(void *pParam) {
    SZEbookReader *pCaller = (SZEbookReader *) (pParam);
    pCaller->m_isReturn = false;
    pCaller->CalcMuciPageNumberRange();
    END_THREAD();
    return 0;
}

SZEbookReader::SZEbookReader() {
//	m_curPageWordInfo = NULL;
    ScreenRatio = 2.0;
    m_calcVer = 1;
    m_curColorIndex = 0;
    m_isNeedCalcProcess = true;
    INITIALIZE_MUTEX(&m_LockBufferVContent);
    INITIALIZE_MUTEX(&m_lockAllPageInfo);
    INITIALIZE_MUTEX(&m_lockCalcCurHtml);
    initWithopp();
}

void SZEbookReader::initWithopp() {
    m_curFileType = FILETYPE_EPUB;

    Cbkhost = NULL;
    CbkFunction = NULL;
    m_iCanFlip = -1;
    m_status = 0;
    m_AllPageNum = 0;
    m_CurrentPageNum = 0;
    m_smallCurrentPageNum = 0;
    m_OutDevW = 0;
    m_OutDevH = 0;
    m_ZoomRatio = 0;
    m_stop = false;
    m_outStop = false;
    m_isNeedCalcPage = true;
    m_IsAutoCalcVert = true;
    m_bookNotes.clear();
    m_nVertSpac = 1;
    m_IsDrawing = false;
    m_ZoomSpan = 2;

    m_bookWordTotle = 0;
    m_bookCalcCurWordNumber = 0;

    m_nFormat = DT_EX_LEFT | DT_EX_TOP/*|DT_EX_UNDERLINE*/;
    m_bInit = false;
    m_bHaveRecord = false;
    m_bIsBookOpen = false;
    m_bEableBG = false;
    m_clrForeG = SZImageBase::MakeRGBQuad(0, 0, 0);
    m_clrBackG = SZImageBase::MakeRGBQuad(255, 255, 255);
    memset(m_strEBookFilePath, 0, 256);
    memset(m_strBgFilePath, 0, 256);

    strcpy(m_strfontname, "simhei");
    strcpy(m_strfontFilePath,
           "g:\\testdata\\fonts\\wqywmh.ttc");///Users/shaozhen/Library/Application Support/iPhone Simulator/5.0/Applications/5BBE4002-772C-48C5-9191-159804C66591/Control Fun.app/
    strcpy(m_strEfontname, "times");
    strcpy(m_strEfontFilePath, "g:\\testdata\\fonts\\times.ttf");

    ClearCurPageWordInfo();
    ClearCurPageMultimedia(m_curPageMultimedia);
    myRGBQUAD color;
    color.rgbRed = 0xff;
    color.rgbGreen = 0xff;
    color.rgbBlue = 0x00;//��
    m_vColor.push_back(color);
    color.rgbRed = 0xff;
    color.rgbGreen = 0x45;
    color.rgbBlue = 0x00;//��
    m_vColor.push_back(color);
    color.rgbRed = 0x7f;
    color.rgbGreen = 0xff;
    color.rgbBlue = 0x00;//��
    m_vColor.push_back(color);
    color.rgbRed = 0x9f;
    color.rgbGreen = 0xff;
    color.rgbBlue = 0xff;//��
    m_vColor.push_back(color);
    color.rgbRed = 0xff;
    color.rgbGreen = 0x83;
    color.rgbBlue = 0xfa;//��
    m_vColor.push_back(color);
//	INITIALIZE_MUTEX(&g_Mutex);

#ifndef _WIN32
    pthread_cond_init(&g_cond, NULL);
    INITIALIZE_MUTEX(&g_ClacpageMutex);
#endif

    m_HighLightColorType = AutoChange;

    m_isReturn = true;
    m_stop = false;
    m_isCalcFinish = false;

    m_currentRecord.vfileID = 0;
    m_currentRecord.contentNumber = 0;
    m_currentRecord.offer = 0;

    m_currentPageRecordFileDir = "";
    m_contentText = NULL;

    m_ScreenType = NormalScreen;
    m_isNeedCalcMuciRange = true;

    m_isBuffPage = false;
    //LOGI("EpubParser_SZEbookReader: init end.");
}

bool SZEbookReader::SaveCurrentPage2Bmp(const char *BMPfilePathName) {
    return m_SZImageCurrent.writeBMPFile(BMPfilePathName);
}

SZEbookReader::~SZEbookReader() {
    CloseEbook();
    ReleaseSource();
    m_vColor.clear();
    DESTROY_MUTEX(&m_LockBufferVContent);
    DESTROY_MUTEX(&m_lockAllPageInfo);
    DESTROY_MUTEX(&m_lockCalcCurHtml);
}

bool SZEbookReader::SetDefaultZoom() {
    if (m_ZoomRatio != 0) {
        m_ZoomRatio = 0;
        UpdateCurNoteToBookNote();
        return ReCalcPage();
    }
    return true;
}

int SZEbookReader::GetZoomRatio() {
    return m_ZoomRatio / m_ZoomSpan;
}

bool SZEbookReader::SetZoomRatio(int zoom) {
    if (!m_bIsBookOpen) {
        //m_ZoomRatio=zoom;
        m_ZoomRatio = zoom * m_ZoomSpan;
        return true;
    }

    if ((m_ZoomRatio / m_ZoomSpan) != zoom) {
        m_ZoomRatio = zoom * m_ZoomSpan;
        UpdateCurNoteToBookNote();
        return ReCalcPage();
    }
}

bool SZEbookReader::SetZoomBig() {
    m_ZoomRatio += m_ZoomSpan;
    UpdateCurNoteToBookNote();
    return ReCalcPage();

}

bool SZEbookReader::SetZoomSmall() {

    m_ZoomRatio -= m_ZoomSpan;
    UpdateCurNoteToBookNote();
    return ReCalcPage();

}

bool SZEbookReader::SetBgFilePath(const char *BMPfilePathName) {
    if (BMPfilePathName == NULL)
        return false;
    int strlength = strlen(BMPfilePathName);
    if (strlength > 0 && strlength <= 255) {
        strcpy(m_strBgFilePath, BMPfilePathName);
        return EableBG(true);
    }
    return false;
}

void SZEbookReader::SetClrBackG(myRGBQUAD clr) {
    m_clrBackG = clr;
    MakeBackgroudImage(m_clrBackG);
    m_bEableBG = false;
}

bool SZEbookReader::EableBG(bool bEnable) {
    if (!m_bInit)
        return true;

    if (bEnable) {
        if (strlen(m_strBgFilePath) <= 3) {
            return false;
        }

        bool rt = MakeBackgroudImage(m_strBgFilePath);
        if (rt)
            m_bEableBG = true;
        return rt;
    } else {
        MakeBackgroudImage(m_clrBackG);
        m_bEableBG = false;
        return true;
    }
}

bool SZEbookReader::Init(int iWidth, int iHeight,/*int zoom,*/int screenType) {
    if (m_bInit) {//Ӧ�ô�ʱ����DispOutChage,�������ٴ�Init
        return false;
    }

    if (iWidth <= 0 || iHeight <= 0) {
        return false;
    }
    m_OutDevW = iWidth;
    m_OutDevH = iHeight;
    if (screenType != NormalScreen && screenType != HDScreen) {
        m_ScreenType = NormalScreen;
    } else
        m_ScreenType = screenType;

    if (!m_SZImageBackgroud.Init(iWidth, iHeight)) {
        m_status = -1;
        return false;
    }

    if (!m_SZImageCurrent.Init(iWidth, iHeight)) {
        m_status = -1;
        return false;
    }

    if (!m_SZImageCashe.Init(iWidth, iHeight)) {
        m_status = -1;
        return false;
    }


    m_myRender.Init(iWidth, iHeight, m_nFormat, m_clrForeG);

    if (strlen(m_strBgFilePath) > 3) {
        if (MakeBackgroudImage(m_strBgFilePath)) {
            m_bEableBG = true;
        } else {
            MakeBackgroudImage(m_clrBackG);
            m_bEableBG = false;
        }
    } else
        MakeBackgroudImage(m_clrBackG);

    if (!fontManger.Init(m_strfontFilePath)) {
        return false;
    }
    if (!E_fontManger.Init(m_strEfontFilePath)) {
        return false;
    }

    m_bInit = true;

    /*m_ZoomRatio = zoom * m_ZoomSpan;*/
    return m_bInit;
}

void SZEbookReader::ReleaseSource() {
}

bool SZEbookReader::ResampleImg(SZImageBase *&pimage, int newWidth, int newHeight) {
#if defined (ANDROID22)
    BWImageScalingAlgorithm *pImgScal = NULL;
#else
    BWImageScalingAlgorithm2* pImgScal=NULL;
#endif
    int nbit = pimage->GetBitCount();
    long lWidth = pimage->GetWidth();
    long lHeight = pimage->GetHeight();
    bool bInit = false;
    bool bB0W1 = false;
    switch (nbit) {
        case 1:
#if defined (ANDROID22)
            pImgScal = new BWImageScalingAlgorithm();
#else
            pImgScal = new BWImageScalingAlgorithm2();
#endif

            bInit = pImgScal->Init(pimage->GetBits(), lWidth, lHeight, 1, bB0W1, newWidth,
                                   newHeight, 1);
            break;
        case 8:
            pImgScal = new Bit8ImageScalingAlgorithm();
            bInit = pImgScal->Init(pimage->GetBits(), lWidth, lHeight, newWidth, newHeight, 1);
            break;
        case 24:
        case 32:
            pImgScal = new Bit24ImageScalingAlgorithm();
            bInit = pImgScal->Init(pimage->GetBits(), lWidth, lHeight, newWidth, newHeight, 1);
            break;
        default:
            return false;


            break;
    }

    if (!bInit) {
        delete pImgScal;
        pImgScal = NULL;
        return false;
    }

#if defined (ANDROID22)
    if (!pImgScal->DoZoom1(true))
#else
        if (!pImgScal->DoZoom(true))
#endif
    {
        delete pImgScal;
        pImgScal = NULL;
        return false;
    }

    delete pimage;
    pimage = NULL;
    switch (nbit) {
        case 1:
            pimage = new SZImageBit1();
            break;
        case 4:
            pimage = new SZImageBit4();
            break;
        case 8:
            pimage = new SZImageBit8();
            break;
        case 24:

            pimage = new SZImageBit24();
            break;
        case 32:
            pimage = new SZImageBit32();
            break;
    }
    if (!pimage) {
        delete pImgScal;
        pImgScal = NULL;
        return false;
    }

    pimage->Init(newWidth, newHeight);
    pimage->SetBits(pImgScal->GetBits(), 0, pImgScal->GetImageSize());
    delete pImgScal;
    pImgScal = NULL;
    return true;
}

void SZEbookReader::CloseEbook() {
    if (m_bIsBookOpen) {
        m_stop = true;
        bookreader.Close();
        m_status = 0;
        m_AllPageNum = 0;
        m_CurrentPageNum = 0;
        m_smallCurrentPageNum = 0;

        m_bHaveRecord = false;
        m_bIsBookOpen = false;

        //m_currentvcontent.clear();
        //	ClearVContent(m_currentvcontent);
        m_vtCurrntFileInfo.clear();
        m_mapAllPageInfo.clear();
        while (!m_bufferDrawPage.empty()) {
            m_bufferDrawPage.pop();
        }

        m_bookNotes.clear();
        m_curPageNotes.Notes.clear();
        m_bInit = false;
        ClearCurPageWordInfo();
        ClearCurPageMultimedia(m_curPageMultimedia);
        ClearBuferVContent();
        //clear font
        //fontManger.End();
        //E_fontManger.End();


        if (m_contentText) {
            delete[] m_contentText;
            m_contentText = NULL;
        }
        initWithopp();
        m_bookWordTotle = m_bookCalcCurWordNumber = 0;
    }
    m_bookNotes.clear();

}

bool SZEbookReader::DispOutChage(int iWidth, int iHeight) {
    UpdateCurNoteToBookNote();
    if (!m_bInit)
        return false;
    m_stop = true;
    while (m_IsDrawing) {
        mySleep(10);
    }
    while (!m_isReturn) {
        mySleep(10);
    }
    m_stop = false;
    /* if (m_ScreenType == HDScreen) {
        double temp = (double)iWidth * ScreenRatio;
        iWidth = (int)temp;
        temp = (double)iHeight * ScreenRatio;
        iHeight = (int)temp;
    }*/

    if (m_OutDevW == iWidth && m_OutDevH == iHeight) {
        return true;
    }

    if (!m_SZImageBackgroud.Init(iWidth, iHeight)) {
        m_status = -1;
        return false;
    }

    if (!m_SZImageCurrent.Init(iWidth, iHeight)) {
        m_status = -1;
        return false;
    }

    if (!m_SZImageCashe.Init(iWidth, iHeight)) {
        m_status = -1;
        return false;
    }

    m_myRender.Init(iWidth, iHeight, m_nFormat, m_clrForeG);

    if (strlen(m_strBgFilePath) > 3) {
        if (MakeBackgroudImage(m_strBgFilePath)) {
            m_bEableBG = true;
        } else {
            MakeBackgroudImage(m_clrBackG);
            m_bEableBG = false;
        }
    } else
        MakeBackgroudImage(m_clrBackG);

    m_OutDevW = iWidth;
    m_OutDevH = iHeight;

    if (m_bIsBookOpen)//���Ѿ��򿪣����¼���
    {
        m_bHaveRecord = true;
        m_lastBookRecord = m_currentRecord;
        m_iCanFlip = -1;
        m_smallCurrentPageNum = 0;
        if (calcPage()) {
            m_status = 1;
            return true;
        } else {
            m_status = -9;
            return false;
        }
    } else return 1;
}

void SZEbookReader::readLastCalcPageRecord() {
    m_AllPageNum = 0;
    m_isCalcFinish = false;

    m_isNeedCalcPage = true;
    m_isNeedCalcMuciRange = true;
    m_mapAllPageInfo.clear();
    LOGI("SZEbookReader_readLastCalcPageRecord: start 1.");
    if (m_currentPageRecordFileDir.empty()) {
        return;
    }
    char filePath[1024];
    memset(filePath, 0, 1024);
    sprintf(filePath, "%s%dc%d %s %s %d %d %d %d %d %d %d %d %d.%s",
            m_currentPageRecordFileDir.c_str(),
            m_OutDevW, m_OutDevH, m_strfontname, m_strEfontname, m_ZoomRatio, GetMrginBottom(),
            GetMrginLeft(),
            GetMrginRight(), GetMrginTop(), GetTabSize(), GetVertSpac(), GetWordSpac(), m_calcVer,
            FileType);
    FILE *recordFile = fopen(filePath, "r");
    LOGI("SZEbookReader_readLastCalcPageRecord | filePath:%s", filePath);
    char buf[30];
    memset(buf, 0, 30);
    if (recordFile) {
        Vt_myBookRecord *pVrecord = new Vt_myBookRecord();
        myBookRecord myrecord;
        int fileId, lastfileid;
        lastfileid = 0;
        while (fgets(buf, 30, recordFile)) {
            char *p = strtok(buf, " ");
            if (p) {
                fileId = atoi(p);
                myrecord.vfileID = fileId;
                p = strtok(NULL, " ");
                if (p) {
                    myrecord.contentNumber = atoi(p);
                    p = strtok(NULL, " ");
                    if (p) {
                        myrecord.offer = atoi(p);
                    }
                }
            }
            m_AllPageNum++;
            if (myrecord.vfileID == m_currentRecord.vfileID &&
                myrecord.contentNumber == m_currentRecord.contentNumber &&
                myrecord.offer == m_currentRecord.offer) {
                m_CurrentPageNum = m_AllPageNum;
            }
            //LOGI("SZEbookReader_readLastCalcPageRecord | record: %d %d %d", myrecord.vfileID , myrecord.contentNumber, myrecord.offer);
            if (fileId != lastfileid) {
                m_mapAllPageInfo.insert(pair<int, Vt_myBookRecord>(lastfileid, *pVrecord));
                delete pVrecord;
                pVrecord = new Vt_myBookRecord();
                lastfileid = fileId;
            }
            pVrecord->push_back(myrecord);
        }
        //LOGI("SZEbookReader_readLastCalcPageRecord | pVrecord size: %d", pVrecord.size());
        if (pVrecord->size() > 0) {
            m_mapAllPageInfo.insert(pair<int, Vt_myBookRecord>(fileId, *pVrecord));
        }
        //LOGI("SZEbookReader_readLastCalcPageRecord: while end.");
        delete pVrecord;

        m_isNeedCalcPage = false;
        m_isCalcFinish = true;
        if (!(m_currentRecord.vfileID == 0 && m_currentRecord.contentNumber == 0 &&
              m_currentRecord.offer == 0)) {
            //�õ���ǰҳ��������Ϣ
            long lSize;
            SZImageBit24 tempImage;
            t_PageOutData temp;
            GetPageByRecord(m_currentRecord, tempImage, lSize, temp, false, false);

        }
        fclose(recordFile);
//LOGI("SZEbookReader_readLastCalcPageRecord: start 3.");
        ReadLastMuciPageRange();
    }
//LOGI("SZEbookReader_readLastCalcPageRecord: end.");
}

bool SZEbookReader::ReCalcPage() {
//	if(1==m_status)
//	{
    while (!m_bufferDrawPage.empty()) {
        m_bufferDrawPage.pop();
    }
    m_bHaveRecord = true;
    m_lastBookRecord = m_currentRecord;
    //	AfxMessageBox(log);
    m_iCanFlip = -1;
    m_smallCurrentPageNum = 0;
    if (calcPage()) {
        //���Ƶ�һҳ
        m_status = 1;
        return true;
    } else {
        m_status = -99;
        return false;
    }
//	}
//	return false;
}

bool SZEbookReader::OpenEbook(const char *file) {

    m_bHaveRecord = false;
    m_isNeedCalcPage = true;
    CloseEbook();
    m_CurrentPageNum = 1;
    if (bookreader.OpenFile((char *) file) != 0) {
        m_status = -1;
        return false;
    }

    m_curFileType = bookreader.m_curFileType;
    m_curtxtfileencodetype = bookreader.m_book->m_curFileEncodingtype;
    m_bIsBookOpen = true;

    m_bookNotesMaxId = 0;
    m_iCanFlip = -1;
    if (calcPage()) {
        m_status = 1;
        return true;
    } else {
        m_status = -9;
        return false;
    }
    return true;
}

unsigned int SZEbookReader::GetFileCount() {
    return bookreader.m_book->m_bookSpine->GetVSpine().size();
}


unsigned int SZEbookReader::GetFilePageCount(unsigned int fileId) {
    unsigned int smallPageCount = 0;
    vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
    if (vfile.size() == 0 || fileId >= vfile.size()) {
        return 0;
    }

    if (!m_isCalcFinish) {
        LOCK_MUTEX(&m_lockAllPageInfo);
        Map_myBookPageInfo::iterator finder = m_mapAllPageInfo.find(fileId);
        if (finder != m_mapAllPageInfo.end()) {
            UNLOCK_MUTEX(&m_lockAllPageInfo);
            smallPageCount = finder->second.size();
        } else {
            UNLOCK_MUTEX(&m_lockAllPageInfo);
            CZEpubReader tempEpubReader;
            vector<Content *> currentvcontent;
            Vt_myBookRecord vtCurrntFileInfo;

            tempEpubReader.SetEpubOpp(bookreader.m_epubFile);
            if (IsCalcFileVContent(fileId)) {
                LOCK_MUTEX(&m_LockBufferVContent);
                currentvcontent = m_mBufferVContent[fileId];
                UNLOCK_MUTEX(&m_LockBufferVContent);
            } else {
                int re = tempEpubReader.ReadFileByFilePath(vfile[fileId]);
                if (re == OPENZIPFILE_DATA_ERROR || re == OPENZIPFILE_NOT_EXIST) {
                    return 0;
                }

                if (tempEpubReader.m_book->m_vContent.size() <= 0) {
                    return 0;
                }

                PushVcontentIntoBuffer(fileId, tempEpubReader.m_book->m_vContent);
                currentvcontent = tempEpubReader.m_book->m_vContent;
            }

            if (RenderContentEx(vtCurrntFileInfo, currentvcontent, fileId) != 1) {
                return 0;
            }
            LOCK_MUTEX(&m_lockAllPageInfo);
            m_mapAllPageInfo.insert(pair<int, Vt_myBookRecord>(fileId, vtCurrntFileInfo));
            UNLOCK_MUTEX(&m_lockAllPageInfo);
            smallPageCount = vtCurrntFileInfo.size();
        }
    } else {
        LOCK_MUTEX(&m_lockAllPageInfo);
        Map_myBookPageInfo::iterator finder = m_mapAllPageInfo.find(fileId);
        UNLOCK_MUTEX(&m_lockAllPageInfo);
        if (finder != m_mapAllPageInfo.end()) {
            smallPageCount = finder->second.size();
        }
    }
    return smallPageCount;
}

void *SZEbookReader::GetPageByFileSmallPageNumber(unsigned int fileId, unsigned int smallPageNumber,
                                                  SZImageBit24 &imageCur, long &imageSize,
                                                  t_PageOutData &pageOutData) {
    LOCK_MUTEX(&m_lockAllPageInfo);
    Map_myBookPageInfoIterator finder = m_mapAllPageInfo.find(fileId);
    UNLOCK_MUTEX(&m_lockAllPageInfo);

    if (finder != m_mapAllPageInfo.end() && finder->second.size() >= smallPageNumber &&
        smallPageNumber > 0) {
        myBookRecord bookrecord = finder->second[smallPageNumber - 1];
        return DrawCurrentPage(imageCur, imageSize, pageOutData);
    }
    return NULL;
}

unsigned int SZEbookReader::GetfileIdByPageNumber(unsigned int pageNumber) {
    int fileId = -1;
    int page = 0;
    map<int, Vt_myBookRecord>::iterator itor;
    for (itor = m_mapAllPageInfo.begin(); itor != m_mapAllPageInfo.end(); itor++) {
        page += itor->second.size();
        if (pageNumber <= page) {
            fileId = itor->first;
            break;
        }
    }
    return fileId;
}

unsigned int SZEbookReader::GetSmallPageNumberByPageNumber(unsigned int pageNumber) {
    int smallPageNumber = -1;
    int page = 0;
    map<int, Vt_myBookRecord>::iterator itor;
    for (itor = m_mapAllPageInfo.begin(); itor != m_mapAllPageInfo.end(); itor++) {
        page += itor->second.size();
        if (pageNumber <= page) {
            page -= itor->second.size();
            smallPageNumber = pageNumber - page;
            break;
        }
    }
    return smallPageNumber;
}

unsigned int SZEbookReader::GetSmallPageNumberByRecord(myBookRecord record) {
    unsigned int smallPageNumber = -1;
    vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
    if (record.vfileID >= vfile.size()) {
        return -1;
    }
    if (!m_isCalcFinish) {
        int re;
        if (m_curFileType == FILETYPE_TXT) {
            m_currentvcontent = bookreader.m_book->m_vContent;
        } else {
            if (m_currenthtml != vfile[record.vfileID] ||
                ((m_currenthtml == vfile[record.vfileID]) && m_vtCurrntFileInfo.size() == 0)) {
                m_currenthtml = vfile[record.vfileID];
                re = calcCurrentHtml(record.vfileID, vfile[record.vfileID], false, false);
            } else
                re = 1;
            if (re > 0) {
                int vtFileInfo = m_vtCurrntFileInfo.size();
                for (int i = 0; i < vtFileInfo; i++) {
                    myBookRecord temp = m_vtCurrntFileInfo[i];
                    if ((m_vtCurrntFileInfo[i].contentNumber == record.contentNumber) &&
                        abs(m_vtCurrntFileInfo[i].offer - record.offer) <= 1) {

                        smallPageNumber = i;
                        break;
                    }
                }
            }
        }
        return smallPageNumber;
    } else {
        int page = 0;
        map<int, Vt_myBookRecord>::iterator itor;
        for (itor = m_mapAllPageInfo.begin(); itor != m_mapAllPageInfo.end(); itor++) {
            if (itor->first == record.vfileID) {
                Vt_myBookRecord vtCurrntFileInfo = itor->second;
                int vtFileInfo = vtCurrntFileInfo.size();
                if (vtFileInfo <= 0) {
                    return -1;
                }

                for (int i = 0; i < vtFileInfo; i++) {
                    page += 1;
                    myBookRecord temp = vtCurrntFileInfo[i];
                    if ((temp.contentNumber == record.contentNumber) &&
                        abs(temp.offer - record.offer) <= 1) {
                        return i;
                    }

                    if (i < (vtFileInfo - 1)) {
                        if (record > vtCurrntFileInfo[i] && record < vtCurrntFileInfo[i + 1]) {
                            return i;
                        }
                    } else {
                        return i;
                    }
                }

            } else {
                page += itor->second.size();
            }
        }
    }
    return smallPageNumber;
}

myBookRecord SZEbookReader::GetRecordByMuciIndex(int muciIndex) {
    myBookRecord record;
    int i;
    if (1 == m_status) {
        int conut = 0;
        map<int, ChapterLevel>::iterator pos2;
        map<int, ChapterLevel> orderchapter = bookreader.m_book->m_bookChapter->GetMapOrderChapter();
        for (pos2 = orderchapter.begin(); pos2 != orderchapter.end(); pos2++) {
            if (pos2->first == muciIndex) {
                vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
                Record infor = bookreader.m_book->GetChapterInforFromChapterOrder(pos2->first);
                for (i = 0; i < vfile.size(); i++) {
                    if (vfile[i] == infor.file) {
                        record.vfileID = i;
                        record.contentNumber = infor.contentNumber;//0;//
                        record.offer = infor.offer;

                        return record;
                    }
                }
            }
            conut++;
        }
    }
}

bool SZEbookReader::calcPage() {
    //��֮ǰûͣ���̱߳��ֹͣ
    m_stop = true;
    m_CalcRender.m_iStop = true;
    while (!m_isReturn) {
        mySleep(10);
    }
    m_stop = false;
    m_CalcRender.m_iStop = false;

    //��ȡһ�¼�¼�ļ����Ƿ���Ҫ����ҳ��
    readLastCalcPageRecord();
    //LOGI("SZEbookReader_calcPage: start thread 2.");
    m_calcpagehThread = CREATE_THREAD(threadstartFun, (void *) this);
    return true;
}

int SZEbookReader::calcCurrentHtml(int fileID, string filePath, bool isNeedRender,
                                   bool isCallbackProgress) {

    LOCK_MUTEX(&m_lockCalcCurHtml);
    if (m_curFileType == FILETYPE_EPUB) {
        if (IsCalcFileVContent(fileID)) {
            LOCK_MUTEX(&m_LockBufferVContent);
            m_currentvcontent = m_mBufferVContent[fileID];
            UNLOCK_MUTEX(&m_LockBufferVContent);
        } else {
            int re = bookreader.ReadFileByFilePath(filePath);
            if (re == OPENZIPFILE_DATA_ERROR) {
                m_status = -72;
                UNLOCK_MUTEX(&m_lockCalcCurHtml);
                return -1;
            } else if (re == 3) {
                UNLOCK_MUTEX(&m_lockCalcCurHtml);
                return 3;
            }
            PushVcontentIntoBuffer(fileID, bookreader.m_book->m_vContent);
            m_currentvcontent = bookreader.m_book->m_vContent;
        }

    } else {
        m_currentvcontent = bookreader.m_book->m_vContent;
    }

    UNLOCK_MUTEX(&m_lockCalcCurHtml);

    if (m_currentvcontent.size() <= 0) {
        return 0;
    }

    if (isNeedRender) {
        m_vtCurrntFileInfo.clear();

        if (RenderContentEx(m_vtCurrntFileInfo, m_currentvcontent, fileID, isCallbackProgress) !=
            1) {
            m_bookCalcCurWordNumber = 0;
            m_status = -83;
            return false;
        }

        if (m_vtCurrntFileInfo.size() <= 0) {
            return 0;
        }
    }

    m_iCanFlip = 0;
    return 1;
}

bool SZEbookReader::calcOtherHtml() {
    char tttttinfo[256];
    CZEpubReader epubreader2;
    Vt_myBookRecord vtCurrntFileInfo;
    vector<Content *> currentvcontent;
    vector<string> vfile;


    if (m_iCanFlip == 1) {
        m_iCanFlip = 0;
    }
    strcpy(tttttinfo, "is calcing pagenumber");
    if (CbkFunction)
        CbkFunction(Cbkhost, tttttinfo, CALLBACK_CALCING);

    if (m_isNeedCalcPage == false) {
        m_status = 1;
        m_iCanFlip = 1;
        if (CbkFunction)
            CbkFunction(Cbkhost, "", CALLBACK_ALLFINISH);
        //����Ŀ¼ҳ�뷶Χ
        // THREAD_HANDLE   hThread = CREATE_THREAD(threadCalcChapterNumberRange,(void *)this);
        CalcMuciPageNumberRange();
        return true;
    }

    if (m_curFileType == FILETYPE_EPUB) {
        epubreader2.SetEpubOpp(bookreader.m_epubFile);
    }
    m_mapAllPageInfo.clear();
    m_AllPageNum = 0;
    vfile = bookreader.m_book->m_bookSpine->GetVSpine();

    /////�Ѽ��㵱ǰhtml�����ŵ��˴�ִ�С�
    if (m_bHaveRecord) {
        m_currenthtml = vfile[m_currentRecord.vfileID];
        if (calcCurrentHtml(m_currentRecord.vfileID, vfile[m_currentRecord.vfileID], true, false) >
            0) {
            int vtFileInfo = m_vtCurrntFileInfo.size();
            for (int i = 0; i < vtFileInfo; i++) {
                if ((m_vtCurrntFileInfo[i].contentNumber == m_currentRecord.contentNumber) &&
                    (m_vtCurrntFileInfo[i].offer == m_currentRecord.offer)) {
                    m_currentRecord = m_vtCurrntFileInfo[i];
                    m_smallCurrentPageNum = i;
                    break;
                }

                if (i < (vtFileInfo - 1)) {
                    if (m_lastBookRecord > m_vtCurrntFileInfo[i] &&
                        m_lastBookRecord < m_vtCurrntFileInfo[i + 1]) {
                        m_currentRecord = m_vtCurrntFileInfo[i];
                        m_smallCurrentPageNum = i;
                        break;
                    }
                } else {
                    m_currentRecord = m_vtCurrntFileInfo[i];
                    m_smallCurrentPageNum = i;
                    break;
                }
            }
            if (CbkFunction) {
                strcpy(tttttinfo, "calc curHtml finish");
                CbkFunction(Cbkhost, tttttinfo, CALLBACK_CURHTMLFINISH);
            }
        } else {
            if (CbkFunction) {
                strcpy(tttttinfo, "calcCurrentHtml error");
                CbkFunction(Cbkhost, tttttinfo, -1);
            }
            return false;
        }

    } else {
        for (int i = 0; i < vfile.size(); i++) {
            m_currentRecord2.vfileID = i;
            if (vfile[i] == m_currenthtml) {
                break;
            }
            m_currenthtml = vfile[i];
            int rt = calcCurrentHtml(i, vfile[i], false);
            if (rt <= 0) {
                if (CbkFunction) {
                    strcpy(tttttinfo, "calcCurrentHtml error");
                    LOGI("SZEbookReader::calcOtherHtml | calcCurrentHtml error  :%d | path:%s", rt,
                         m_currenthtml.c_str());
                    CbkFunction(Cbkhost, tttttinfo, -1);
                }
                return false;
            }
            if (rt > 0) {
                if (rt == OPENZIPFILE_NOT_EXIST) {//�ļ�������
                    continue;
                }
                //m_currentRecord=m_vtCurrntFileInfo[0];
                m_currentRecord.vfileID = i;
                m_currentRecord.contentNumber = 0;
                m_currentRecord.offer = 0;
                m_smallCurrentPageNum = 0;
                if (CbkFunction) {
                    strcpy(tttttinfo, "calc curHtml finish");
                    CbkFunction(Cbkhost, tttttinfo, CALLBACK_CURHTMLFINISH);
                }
                break;
            }
        }
    }
    if (m_stop) {
        if (CbkFunction && m_outStop) {
            strcpy(tttttinfo, "calc stop");
            CbkFunction(Cbkhost, tttttinfo, CALLBACK_STOP);
        }
        return false;
    }

    if (m_isNeedCalcProcess && CalcBookWordTotle(&epubreader2, vfile) <= 0) {
        if (CbkFunction) {
            strcpy(tttttinfo, "calc book word totle error");
            LOGI("SZEbookReader::calcOtherHtml | calc book word totle error ");
            CbkFunction(Cbkhost, tttttinfo, -1);
        }
        return false;
    }

    m_fStep = 1;
    for (int i = 0; i < vfile.size(); i++) {
        if (m_stop) {
            sprintf(tttttinfo, "calc stop");
            if (CbkFunction && m_outStop)
                CbkFunction(Cbkhost, tttttinfo, CALLBACK_STOP);
            return false;
        }
        if (CbkFunction)
            CbkFunction(Cbkhost, tttttinfo, CALLBACK_CURHTMLFINISH);
        if (m_curFileType == FILETYPE_TXT) {
            currentvcontent = bookreader.m_book->m_vContent;
        } else {
            if (IsCalcFileVContent(i)) {
                LOCK_MUTEX(&m_LockBufferVContent);
                currentvcontent = m_mBufferVContent[i];
                UNLOCK_MUTEX(&m_LockBufferVContent);
            } else {
                int re = epubreader2.ReadFileByFilePath(vfile[i]);
                if (re == OPENZIPFILE_DATA_ERROR) {
                    continue;;
                } else if (re == OPENZIPFILE_NOT_EXIST) {//�ļ�������
                    //bookreader.m_book->m_bookSpine->DeleteItem(i);
                    //vfile=bookreader.m_book->m_bookSpine->GetVSpine();
                    continue;
                }
                if (epubreader2.m_book->m_vContent.size() <= 0) {
                    continue;
                }
                PushVcontentIntoBuffer(i, epubreader2.m_book->m_vContent);
                currentvcontent = epubreader2.m_book->m_vContent;
            }
        }
        if (RenderContentEx(vtCurrntFileInfo, currentvcontent, i) != 1) {
            m_status = -83;
            sprintf(tttttinfo, "RenderContentError");
            if (CbkFunction)
                CbkFunction(Cbkhost, tttttinfo, -1);
            return false;
        }
        if (m_stop) {
            if (CbkFunction && m_outStop)
                CbkFunction(Cbkhost, "calc stop", CALLBACK_STOP);
            return false;
        }
        if (CbkFunction)
            CbkFunction(Cbkhost, tttttinfo, CALLBACK_CURHTMLFINISH);
        m_mapAllPageInfo.insert(pair<int, Vt_myBookRecord>(i, vtCurrntFileInfo));
    }
    if (m_curFileType == FILETYPE_EPUB) {
        bookreader.m_book->CopyMapAnchorContent(epubreader2.m_book);
    }

    bool isSuccessOpenRecordFile(true);
    char filePath[1024];
    memset(filePath, 0, 1024);
    char item[30];
    FILE *creatFile = NULL;

    if (m_currentPageRecordFileDir.empty()) {
        isSuccessOpenRecordFile = false;
    } else {
        sprintf(filePath, "%s%dc%d %s %s %d %d %d %d %d %d %d %d %d.%s",
                m_currentPageRecordFileDir.c_str(),
                m_OutDevW, m_OutDevH, m_strfontname, m_strEfontname, m_ZoomRatio, GetMrginBottom(),
                GetMrginLeft(),
                GetMrginRight(), GetMrginTop(), GetTabSize(), GetVertSpac(), GetWordSpac(),
                m_calcVer, FileType);
        //sprintf(filePath,"%s1.txt",m_currentPageRecordFileDir.c_str());
        creatFile = fopen(filePath, "w+");
        if (creatFile == NULL) {
            isSuccessOpenRecordFile = false;
        }
    }
    m_AllPageNum = 0;
    map<int, Vt_myBookRecord>::iterator itor;
    for (itor = m_mapAllPageInfo.begin(); itor != m_mapAllPageInfo.end(); itor++) {
        if (itor->first == m_currentRecord.vfileID) {
            myBookRecord tempRecord;
            for (int i = 0; i < itor->second.size(); i++) {
                tempRecord = itor->second[i];
                m_AllPageNum += 1;
                if (tempRecord.contentNumber == m_currentRecord.contentNumber &&
                    tempRecord.offer == m_currentRecord.offer) {
                    m_CurrentPageNum = m_AllPageNum;
                }
            }
        } else {
            m_AllPageNum += itor->second.size();
        }
        //�����������Ϣ
        //����Ҫ�жϼ�¼�ļ����Ƿ���ڣ���Ϊ���ߵ�����ط������϶��ǵ�һ�μ��㡣����ֱ�ӽ��в��뱣��.
        if (isSuccessOpenRecordFile) {
            myBookRecord tempRecord;
            for (int i = 0; i < itor->second.size(); i++) {
                tempRecord = itor->second[i];
                memset(item, 0, 30);
                sprintf(item, "%d %d %d\n", tempRecord.vfileID, tempRecord.contentNumber,
                        tempRecord.offer);
                fwrite(item, 1, strlen(item), creatFile);
            }
        }
    }
    if (isSuccessOpenRecordFile) {
        fclose(creatFile);
    }
    m_iCanFlip = 1;
    m_isCalcFinish = true;
    strcpy(tttttinfo, "");
    if (CbkFunction)
        CbkFunction(Cbkhost, tttttinfo, CALLBACK_ALLFINISH);
    // THREAD_HANDLE   hThread2 = CREATE_THREAD(threadCalcChapterNumberRange,(void *)this);
    if (1/*m_curFileType == FILETYPE_EPUB*/) {
        CalcMuciPageNumberRange();
    } else {
        if (CbkFunction)
            CbkFunction(Cbkhost, "", CALLBACK_OUTLINE_PAGERANGE);
    }

    return true;
}

void *
SZEbookReader::ReadCorrespondPageByMuciIndex(int MuciIndex, SZImageBit24 &imageCur, long &imageSize,
                                             t_PageOutData &pageOutData) {

    pageOutData.pwi.lines.clear();
    pageOutData.pwi.images.clear();
    pageOutData.pmm.clear();

    int i;
    if (1 == m_status) {

        int conut = 0;
        map<int, ChapterLevel>::iterator pos2;
        map<int, ChapterLevel> orderchapter = bookreader.m_book->m_bookChapter->GetMapOrderChapter();
        for (pos2 = orderchapter.begin(); pos2 != orderchapter.end(); pos2++) {
            if (pos2->first == MuciIndex) {
                vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
                Record infor = bookreader.m_book->GetChapterInforFromChapterOrder(pos2->first);
                for (i = 0; i < vfile.size(); i++) {
                    if (vfile[i] == infor.file) {
                        m_lastBookRecord.vfileID = i;
                        m_lastBookRecord.contentNumber = infor.contentNumber;//0;//
                        m_lastBookRecord.offer = infor.offer;

                        return GetPageByRecord(m_lastBookRecord, imageCur, imageSize, pageOutData,
                                               true);
                    }
                }
                return NULL;
            }
            conut++;
        }
    }
    return NULL;
}

long SZEbookReader::GetSize() const {
    return m_SZImageCurrent.GetSize();
}

void *SZEbookReader::GetBMP() const {
    return m_SZImageCurrent.GetBMP();
}

void *SZEbookReader::GotoPage(int PageNum, SZImageBit24 &imageCur, long &imageSize,
                              t_PageOutData &pageOutData, int flag) {

    pageOutData.pwi.lines.clear();
    pageOutData.pwi.images.clear();
    pageOutData.pmm.clear();
    LOGI("SZEbookReader.GotoPage start|  pageNum:%d  | pageCount:%d", PageNum, m_AllPageNum);
    if ((1 == m_status) && (m_iCanFlip == 1)) {
        vector<Content *> vContent;
        if (PageNum < 1 || PageNum > m_AllPageNum) {
            return NULL;
        }
        //  start = mach_absolute_time ();
        UpdateCurNoteToBookNote();
        /* uint64_t end = mach_absolute_time ();
        uint64_t elapsed = end - start;
        uint64_t nanos = elapsed * info.numer / info.denom;
        printf("UpdateCurNoteToBookNote  %f\n",(float)nanos / NSEC_PER_SEC);*/

        int page = 0;
        map<int, Vt_myBookRecord>::iterator itor;
        for (itor = m_mapAllPageInfo.begin(); itor != m_mapAllPageInfo.end(); itor++) {
            page += itor->second.size();
            LOGI("SZEbookReader.GotoPage |pageNum:%d | page:%d", PageNum, page);
            if (PageNum <= page) {
                LOGI("SZEbookReader.GotoPage |find html | pageNum:%d | page:%d", PageNum, page);
                page -= itor->second.size();
                for (int i = 0; i < itor->second.size(); i++) {
                    page += 1;
                    if (page == PageNum) {
                        LOGI("SZEbookReader.GotoPage |find page | pageNum:%d | page:%d | i:%d | vtFileInfo:%d",
                             PageNum, page, i, itor->second.size());
                        m_CurrentPageNum = PageNum;
                        m_vtCurrntFileInfo = itor->second;
                        m_currentRecord = m_vtCurrntFileInfo[i];
                        //vector<string>		vfile=bookreader.m_book->m_bookSpine->GetVSpine();
                        LOGI("SZEbookReader.GotoPage | find page | record:fileid:%d  contentNumber:%d  offset:%d ",
                             m_currentRecord.vfileID, m_currentRecord.contentNumber,
                             m_currentRecord.offer);
                        if (m_vtCurrntFileInfo.size() > 1) {
                            LOGI("SZEbookReader.GotoPage | m_vtCurrntFileInfo 0 :fileid:%d  contentNumber:%d  offset:%d ",
                                 m_vtCurrntFileInfo[0].vfileID, m_vtCurrntFileInfo[0].contentNumber,
                                 m_vtCurrntFileInfo[0].offer);
                            LOGI("SZEbookReader.GotoPage | m_vtCurrntFileInfo 1 :fileid:%d  contentNumber:%d  offset:%d ",
                                 m_vtCurrntFileInfo[1].vfileID, m_vtCurrntFileInfo[1].contentNumber,
                                 m_vtCurrntFileInfo[1].offer);
                        }

                        /*int fileid = m_currentRecord.vfileID;
						if (m_curFileType == FILETYPE_EPUB)
						{
							while(1)
							{
								m_currenthtml=vfile[fileid];

								if (IsCalcFileVContent(fileid))
								{
									LOCK_MUTEX(&m_LockBufferVContent);
									vContent = m_mBufferVContent[fileid];
									UNLOCK_MUTEX(&m_LockBufferVContent);
									break;
								}
								else
								{
									int re = bookreader.ReadFileByFilePath(m_currenthtml);
									if (re == OPENZIPFILE_DATA_ERROR)
									{
										m_status=-92;
										return 	NULL;
									}
									else if (re == OPENZIPFILE_NOT_EXIST)
									{
										//fileid = flag==1?fileid+1:fileid-1;
										bookreader.m_book->m_bookSpine->DeleteItem(fileid);
										vfile=bookreader.m_book->m_bookSpine->GetVSpine();
										continue;
									}
									else
									{
										if (bookreader.m_book->m_vContent.size()<=0)
										{
											continue;
										}
										PushVcontentIntoBuffer(fileid,bookreader.m_book->m_vContent);
										vContent = bookreader.m_book->m_vContent;
										break;
									}

								}
							}
						}
						else
						{
							vContent = bookreader.m_book->m_vContent;
						}
					//	m_currentvcontent = bookreader.m_book->m_vContent;
						if (vContent.size()<=0)
						{
							m_status=-93;
							return 	NULL;
						}*/
                        // 	long t1 = GetTickCount();
                        LOGI("SZEbookReader.GotoPage | begin draw page");
                        void *p = DrawCurrentPage(imageCur, imageSize, pageOutData,
                                                  &m_currentRecord/*,&vContent*/);
                        LOGI("SZEbookReader.GotoPage | end draw page | p:%p", p);
                        //	long t2 = GetTickCount();
                        //	TRACE("drawCurrentPage use time %ld\n",t2-t1);
                        return p;
                    }
                }
            }
        }
    }
    return NULL;
}

void *SZEbookReader::NextPage(SZImageBit24 &imageCur, long &imageSize, t_PageOutData &pageOutData) {

    pageOutData.pwi.lines.clear();
    pageOutData.pwi.images.clear();
    pageOutData.pmm.clear();
    m_error = othererror;
    if (1 == m_status) {
        vector<Content *> vContent;
        UpdateCurNoteToBookNote();
        if (m_iCanFlip == 1) {
            if (m_CurrentPageNum < (m_AllPageNum)) {
                m_CurrentPageNum++;
                myBookRecord temp;
                m_currentRecord.vfileID = m_beforCalcRecord.vfileID;
                m_currentRecord.contentNumber = m_beforCalcRecord.contentNumber;
                m_currentRecord.offer = m_beforCalcRecord.offer;
                void *imgData = GotoPage(m_CurrentPageNum, imageCur, imageSize, pageOutData, 1);
                return imgData;
            }
        } else {
            vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
            if (m_beforCalcRecord.vfileID >= vfile.size()) {
                m_error = endPage;
                return NULL;
            }
            /*int fileid = m_beforCalcRecord.vfileID;
			if (m_curFileType == FILETYPE_EPUB)
			{
				while(1)
				{
					m_currenthtml=vfile[fileid];
					if (IsCalcFileVContent(fileid))
					{
						LOCK_MUTEX(&m_LockBufferVContent);
						vContent = m_mBufferVContent[fileid];
						UNLOCK_MUTEX(&m_LockBufferVContent);
						break;
					}
					else
					{
						int re = bookreader.ReadFileByFilePath(m_currenthtml);
						if (re == OPENZIPFILE_DATA_ERROR)
						{
							m_status=-92;
							return 	NULL;
						}
						else if (re == 3)
						{
							//fileid++;
							bookreader.m_book->m_bookSpine->DeleteItem(fileid);
							vfile=bookreader.m_book->m_bookSpine->GetVSpine();
							continue;
						}
						else
						{
							if (bookreader.m_book->m_vContent.size()<=0)
							{
								continue;
							}
							PushVcontentIntoBuffer(fileid,bookreader.m_book->m_vContent);
							vContent = bookreader.m_book->m_vContent;
							break;
						}

					}
				}
			}
			else
				vContent = bookreader.m_book->m_vContent;
			if (vContent.size()<=0)
			{
				m_status=-93;
				return 	NULL;
			}
			*/
            m_CurrentPageNum++;
            m_smallCurrentPageNum++;
            myBookRecord temp;
            m_currentRecord.vfileID = m_beforCalcRecord.vfileID;
            m_currentRecord.contentNumber = m_beforCalcRecord.contentNumber;
            m_currentRecord.offer = m_beforCalcRecord.offer;
            void *imgData = DrawCurrentPage(imageCur, imageSize, pageOutData,
                                            &m_currentRecord/*,&vContent*/);
            if (m_currentRecord.contentNumber == 0 && m_currentRecord.offer == 0) {
                m_smallCurrentPageNum = 0;
            }
            return imgData;
        }
    }
    return NULL;
}

void *SZEbookReader::PrevPage(SZImageBit24 &imageCur, long &imageSize, t_PageOutData &pageOutData) {

    pageOutData.pwi.lines.clear();
    pageOutData.pwi.images.clear();
    pageOutData.pmm.clear();
    m_error = othererror;
    if (1 == m_status) {

        vector<Content *> vContent;
        UpdateCurNoteToBookNote();
        if (m_iCanFlip == 1) {//������ҳ��
            if (m_CurrentPageNum > 1) {
                m_CurrentPageNum--;
                return GotoPage(m_CurrentPageNum, imageCur, imageSize, pageOutData);
            }
        } else if (m_iCanFlip == 0) {
            myBookRecord drawPageRecord;
            if (m_curFileType == FILETYPE_EPUB) {
                if (m_vtCurrntFileInfo.size() == 0) {
                    calcHtml(m_currentRecord.vfileID);
                    //m_smallCurrentPageNum = m_vtCurrntFileInfo.size();
                }
                if (m_smallCurrentPageNum == 0 && m_vtCurrntFileInfo[0].vfileID == 0) {
                    m_error = firstPage;
                    return NULL;
                } else if (m_smallCurrentPageNum == 0) {
                    int needCalcHtmlFileId = m_vtCurrntFileInfo[0].vfileID - 1;
                    calcHtml(needCalcHtmlFileId);
                    m_smallCurrentPageNum = m_vtCurrntFileInfo.size();
                }
                /*if (IsCalcFileVContent(m_vtCurrntFileInfo[0].vfileID))
			{
				LOCK_MUTEX(&m_LockBufferVContent);
				vContent = m_mBufferVContent[m_vtCurrntFileInfo[0].vfileID];
				UNLOCK_MUTEX(&m_LockBufferVContent);
			}
			else
			{
				vector<string>	vfile=bookreader.m_book->m_bookSpine->GetVSpine();
				int re = bookreader.ReadFileByFilePath(vfile[m_vtCurrntFileInfo[0].vfileID]);
					if ( re == OPENZIPFILE_DATA_ERROR)
				{
					//m_status=-72;
					return 	NULL;
				}
				else if (re == 3)
				{
					return 	NULL;
				}
				vContent = bookreader.m_book->m_vContent;
			}*/

                m_smallCurrentPageNum--;
                m_CurrentPageNum--;
                drawPageRecord = m_vtCurrntFileInfo[m_smallCurrentPageNum];
            } else {
                if (m_bufferDrawPage.empty()) {
                    m_error = firstPage;
                    return NULL;
                }
                m_bufferDrawPage.pop();
                if (m_bufferDrawPage.empty()) {
                    m_error = firstPage;
                    return NULL;
                }
                drawPageRecord = m_bufferDrawPage.top();
                m_bufferDrawPage.pop();
                vContent = bookreader.m_book->m_vContent;
            }

            void *temp2 = DrawCurrentPage(imageCur, imageSize, pageOutData,
                                          &drawPageRecord/*,&vContent*/);
            return temp2;
        }
    }
    return NULL;
}


bool SZEbookReader::RenderBG() {
    if (m_SZImageCurrent.GetWidth() != m_SZImageBackgroud.GetWidth())
        return false;
    if (m_SZImageCurrent.GetHeight() != m_SZImageBackgroud.GetHeight())
        return false;
    if (m_SZImageCurrent.GetSize() != m_SZImageBackgroud.GetSize())
        return false;

    m_SZImageCurrent.SetBits(m_SZImageBackgroud.GetBits(), 0, m_SZImageBackgroud.GetSize());
    return true;
}

bool SZEbookReader::MakeBackgroudImage(myRGBQUAD clr) {
// 	if (!m_bInit)
// 		return	 false;
    int i, j;

    for (j = m_SZImageBackgroud.GetHeight() - 1; j >= 0; j--) {
        for (i = 0; i < m_SZImageBackgroud.GetWidth(); i++) {
            m_SZImageBackgroud.SetDesPixel(i, j, clr, 255);
        }
    }
    return true;
}

bool SZEbookReader::MakeBackgroudImage(const char *BMPfilePathName) {
    FILE *hFile = fopen(BMPfilePathName, "rb");
    if (hFile == NULL)
        return false;

    myDWORD filelen = 0;
    myBITMAPFILEHEADER bf;
    myBITMAPINFOHEADER bmpHeader;
    fseek(hFile, 0L, SEEK_END);
    filelen = ftell(hFile);

    fseek(hFile, 0, SEEK_SET);
    if (sizeof(bf) != fread(&bf, 1, sizeof(bf), hFile)) {
        fclose(hFile);
        hFile = NULL;
        return false;
    }

    myDWORD bfOffBits = MAKEMYDWORD(bf.bfOffBitslow, bf.bfOffBitshigh);
    myDWORD bfSize = MAKEMYDWORD(bf.bfSizelow, bf.bfSizehigh);

    if ((bf.bfType != 0x4d42) || (bfSize != filelen)) {
        fclose(hFile);
        hFile = NULL;
        return false;
    }

    fseek(hFile, sizeof(bf), SEEK_SET);
    if (sizeof(bmpHeader) != fread(&bmpHeader, 1, sizeof(bmpHeader), hFile)) {
        fclose(hFile);
        hFile = NULL;
        return false;
    }

    bool bsupportBitCount = false;
    if (bmpHeader.biBitCount == 1 || bmpHeader.biBitCount == 4 || bmpHeader.biBitCount == 8 ||
        bmpHeader.biBitCount == 24) {
        bsupportBitCount = true;
    }

    if ((!bsupportBitCount) || (bmpHeader.biHeight < 0) || (bmpHeader.biCompression != BI_RGB)) {
        fclose(hFile);
        hFile = NULL;
        return false;
    }

    SZImageBase *pOrigimg = NULL;
    switch (bmpHeader.biBitCount) {
        case 24 :
            pOrigimg = new SZImageBit24();
            break;
        case 8 :
            pOrigimg = new SZImageBit8();
            break;
        case 4 :
            pOrigimg = new SZImageBit4();
            break;
        case 1 :
            pOrigimg = new SZImageBit1();
            break;
    }

    if (pOrigimg == NULL) {
        fclose(hFile);
        hFile = NULL;
        return false;
    }

    if (!(pOrigimg->Init(bmpHeader.biWidth, bmpHeader.biHeight))) {
        delete pOrigimg;
        pOrigimg = NULL;
        fclose(hFile);
        hFile = NULL;
        return false;
    }

    if (filelen != (myDWORD) (pOrigimg->GetSize())) {
        delete pOrigimg;
        pOrigimg = NULL;
        fclose(hFile);
        hFile = NULL;
        return false;
    }

    fseek(hFile, 0, SEEK_SET);
    if (filelen != fread(pOrigimg->GetBMP(), 1, filelen, hFile)) {
        delete pOrigimg;
        pOrigimg = NULL;
        fclose(hFile);
        hFile = NULL;
        return false;
    }
    fclose(hFile);
    hFile = NULL;

    int i, j, ii, jj, oriImgW, oriImgH;
    oriImgW = pOrigimg->GetWidth();
    oriImgH = pOrigimg->GetHeight();

    for (j = m_OutDevH - 1; j >= 0; j--) {
        for (i = 0; i < m_OutDevW; i++) {
            ii = i % oriImgW;
            jj = j % oriImgH;
            m_SZImageBackgroud.SetDesPixel(i, j, pOrigimg->GetOriPixel(ii, jj), 255);
        }
    }
    delete pOrigimg;
    pOrigimg = NULL;
    return true;
}

pageWordinfo SZEbookReader::GetCurPageWordInfo() {
    for (int i = 0; i < m_curPageWordInfo.lines.size(); i++) {
        int minx(1000000000), miny(100000000), maxHeight(0);
        for (int j = 0; j < m_curPageWordInfo.lines[i].words.size(); j++) {
            if (m_curPageWordInfo.lines[i].words[j].x < minx)
                minx = m_curPageWordInfo.lines[i].words[j].x;

            if (m_curPageWordInfo.lines[i].words[j].y < miny)
                miny = m_curPageWordInfo.lines[i].words[j].y;
            if (m_curPageWordInfo.lines[i].words[j].height > maxHeight) {
                maxHeight = m_curPageWordInfo.lines[i].words[j].height;
            }

        }
        m_curPageWordInfo.lines[i].x = minx;
        m_curPageWordInfo.lines[i].y = miny;
        if (m_curPageWordInfo.lines[i].words.size() > 0) {
            m_curPageWordInfo.lines[i].Width =
                    m_curPageWordInfo.lines[i].words[m_curPageWordInfo.lines[i].words.size() -
                                                     1].x +
                    m_curPageWordInfo.lines[i].words[m_curPageWordInfo.lines[i].words.size() -
                                                     1].width - m_curPageWordInfo.lines[i].x;
            m_curPageWordInfo.lines[i].Height = maxHeight;
        } else {
            m_curPageWordInfo.lines[i].Width = 0;
            m_curPageWordInfo.lines[i].Height = maxHeight;
        }

    }

    if (m_ScreenType == NormalScreen) {
        return m_curPageWordInfo;
    } else {
        for (int i = 0; i < m_curPageWordInfo.lines.size(); i++) {
            m_curPageWordInfo.lines[i].x = (int) ((double) m_curPageWordInfo.lines[i].x /
                                                  ScreenRatio);
            m_curPageWordInfo.lines[i].y = (int) ((double) m_curPageWordInfo.lines[i].y /
                                                  ScreenRatio);
            m_curPageWordInfo.lines[i].Width = (int) ((double) m_curPageWordInfo.lines[i].Width /
                                                      ScreenRatio);
            m_curPageWordInfo.lines[i].Height = (int) ((double) m_curPageWordInfo.lines[i].Height /
                                                       ScreenRatio);
            for (int j = 0; j < m_curPageWordInfo.lines[i].words.size(); j++) {
                m_curPageWordInfo.lines[i].words[j].x = (int) (
                        (double) m_curPageWordInfo.lines[i].words[j].x / ScreenRatio);
                m_curPageWordInfo.lines[i].words[j].y = (int) (
                        (double) m_curPageWordInfo.lines[i].words[j].y / ScreenRatio);
                m_curPageWordInfo.lines[i].words[j].width = (int) (
                        (double) m_curPageWordInfo.lines[i].words[j].width / ScreenRatio);
                m_curPageWordInfo.lines[i].words[j].height = (int) (
                        (double) m_curPageWordInfo.lines[i].words[j].height / ScreenRatio);
            }
        }
        return m_curPageWordInfo;
    }

}

void SZEbookReader::PutBookNotes(Vt_notes booknotes) {
    m_bookNotes = booknotes;
    if (booknotes.size() > 0) {
        m_bookNotesMaxId = booknotes[booknotes.size() - 1].noteId + 1;
    }
}

void *SZEbookReader::DrawCurPageHighLight(SZImageBit24 &imgCur, curPageNote &cpn, long &imagesize) {
    if (cpn.e_contentID < cpn.s_contentID) {
        return NULL;
    }
    if (cpn.e_contentID == cpn.s_contentID && cpn.e_offset < cpn.s_offset) {
        return NULL;
    }

    //curPageNote thiscpn;
    if (m_HighLightColorType == NoChange) {
        cpn.colorIndex = 0;
        m_curColorIndex = 0;
    } else {
        cpn.colorIndex = m_curColorIndex++;
        m_curColorIndex %= m_vColor.size();
    }

    //���ɱ�ҳrender����
    CRender pageRender = m_myRender;
    //����curpagewordinfo ����ͼ
    if (!InitPageBmpByWordInfo(imgCur, &pageRender)) {
        return NULL;
    }
//	imgCur.writeBMPFile("g:\\hi.bmp");

    //����
    cpn.curBookId = m_bookNotesMaxId++;
    cpn.curPageID = m_curPageNotes.maxID++;
    cpn.fileID = m_currentRecord.vfileID;
// 	thiscpn.s_contentID = cpn.s_contentID;
// 	thiscpn.s_offset = cpn.s_offset;
// 	thiscpn.e_contentID = cpn.e_contentID;
// 	thiscpn.e_offset = cpn.e_offset;
    cpn.status = 1;//����
    //thiscpn.type = cpn.type;
    m_curPageNotes.Notes.push_back(cpn);


    Vt_curPageNotes::iterator pos;
    for (pos = m_curPageNotes.Notes.begin(); pos != m_curPageNotes.Notes.end(); pos++) {
        if (pos->status != 3) {
            SetHighLight3(imgCur, &pageRender, pos->s_contentID, pos->s_offset, pos->e_contentID,
                          pos->e_offset, m_vColor[pos->colorIndex]);
        }
    }


#ifdef _USE_BMP
                                                                                                                            imagesize = imgCur.GetSize();
	return imgCur.GetBMP();
#else
    CxImage tmp((BYTE *) imgCur.GetBMP(), imgCur.GetSize(), CXIMAGE_FORMAT_BMP);
    if (tmp.IsValid()) {
        BYTE *buf = NULL;
        tmp.Encode(buf, imagesize, CXIMAGE_FORMAT_PNG);
        return buf;
    } else
        return NULL;
#endif
}

bool SZEbookReader::DeleteCurPageNoteById(SZImageBit24 &imgCur, int id) {
    //���ɱ�ҳrender����
    CRender pageRender = m_myRender;
    if (!InitPageBmpByWordInfo(imgCur, &pageRender)) {
        return false;
    }
    int booknoteID(-1);
    Vt_curPageNotes::iterator pos;
    bool isHaveDelete(false);//ÊÇ·ñÕÒµœÉŸ³ý¶ÔÏó
    for (pos = m_curPageNotes.Notes.begin(); pos != m_curPageNotes.Notes.end(); pos++) {
        if (pos->curBookId == id) {
            isHaveDelete = true;
            if (pos->status == 0 || pos->status == 2) {//Ö®Ç°±£ŽæµÄÒª±êŒÇÉŸ³ý£¬µ±»»Ò³µÄÊ±ºòœøÐÐžüÐÂ
                pos->status = 3;
            } else if (pos->status == 1) {//µ±Ç°Ò³ÔöŒÓµÄÖ±œÓÉŸ³ý
                m_curPageNotes.Notes.erase(pos);
            }
            break;
        }
    }
    if (!isHaveDelete) {
        return false;
    }
//	m_SZImageCurrent.SetBits(m_SZImageCashe.GetBits(),0,m_SZImageCashe.GetSize());
    for (pos = m_curPageNotes.Notes.begin(); pos != m_curPageNotes.Notes.end(); pos++) {
        if (pos->status != 3) {
            SetHighLight3(imgCur, &pageRender, pos->s_contentID, pos->s_offset, pos->e_contentID,
                          pos->e_offset, m_vColor[pos->colorIndex]);
        }
    }
    //m_SZImageCurrent.writeBMPFile( ("c:\\SZDispOut.bmp"));
    return true;
}

bool SZEbookReader::UpdateCurNoteToBookNote() {
    int i, j;
    for (i = 0; i < m_curPageNotes.Notes.size(); i++) {
        curPageNote cpn = m_curPageNotes.Notes[i];

        note booknote;
        booknote.fileId = /*m_curPageWordInfo.fileID*/cpn.fileID;
        booknote.noteId = cpn.curBookId;
        booknote.s_contentID = cpn.s_contentID;
        booknote.s_offset = cpn.s_offset;
        booknote.e_contentID = cpn.e_contentID;
        booknote.e_offset = cpn.e_offset;
        booknote.colorIndex = cpn.colorIndex;
        booknote.type = cpn.type;
        Vt_notes::iterator poss;
        Vt_notes::iterator min, max, pos;
        switch (cpn.status) {
            case 0:
                break;
            case 1://ÔöŒÓÅú×¢
                //booknote.noteId = m_bookNotes.size();
                //ÕÒµœ²åÈëÎ»ÖÃ.
                if (m_bookNotes.size() > 0) {
                    min = m_bookNotes.begin();
                    max = m_bookNotes.end();
                    pos = min;
                    while ((min + 1) < max) {
                        pos = min + (max - min) / 2;
                        note thisnote = *pos;
                        if (thisnote.fileId > booknote.fileId) {
                            max = pos;
                        } else if (thisnote.fileId < booknote.fileId) {
                            min = pos;
                        } else
                            break;
                    }
                    m_bookNotes.insert(pos, booknote);
                } else
                    m_bookNotes.push_back(booknote);
                break;
            case 2://ÐÞžÄÅú×¢
                for (j = 0; j < m_bookNotes.size(); j++) {
                    if (m_bookNotes[j].noteId == cpn.curBookId) {
                        m_bookNotes[j].s_contentID = cpn.s_contentID;
                        m_bookNotes[j].s_offset = cpn.s_offset;
                        m_bookNotes[j].e_contentID = cpn.e_contentID;
                        m_bookNotes[j].e_offset = cpn.e_offset;
                        m_bookNotes[j].colorIndex = cpn.colorIndex;
                        break;
                    }
                }
                break;
            case 3:

                for (poss = m_bookNotes.begin(); poss != m_bookNotes.end(); poss++) {
                    if (poss->noteId == cpn.curBookId) {
                        m_bookNotes.erase(poss);
                        break;
                    }
                }
                break;
            default:
                break;

        }
    }
    return true;
}


bool SZEbookReader::ModifyCurPageNoteById(SZImageBit24 &imgCur, curPageNote &cpn) {
    CRender pageRender = m_myRender;
    if (!InitPageBmpByWordInfo(imgCur, &pageRender)) {
        return false;
    }
    int booknoteID(-1);
    Vt_curPageNotes::iterator pos;
    bool isFind(false);//ÊÇ·ñÕÒµœÐÞžÄ¶ÔÏó
    for (pos = m_curPageNotes.Notes.begin(); pos != m_curPageNotes.Notes.end(); pos++) {
        if (pos->curBookId == cpn.curBookId) {
            isFind = true;
            if (pos->status == 0 || pos->status ==
                                    2) {//Ö®Ç°±£ŽæµÄÒª±êŒÇÐÞžÄ£¬µ±»»Ò³µÄÊ±ºòœøÐÐžüÐÂ//ÕâŽÎÔöŒÓµÄ±ê×¢ÈÔÎªÔöŒÓ
                pos->status = 2;
            }
            pos->s_contentID = cpn.s_contentID;
            pos->s_offset = cpn.s_offset;
            pos->e_contentID = cpn.e_contentID;
            pos->e_offset = cpn.e_offset;
            pos->colorIndex = cpn.colorIndex;
            break;
        }
    }
    if (!isFind) {
        return false;
    }
//	m_SZImageCurrent.SetBits(m_SZImageCashe.GetBits(),0,m_SZImageCashe.GetSize());
    for (pos = m_curPageNotes.Notes.begin(); pos != m_curPageNotes.Notes.end(); pos++) {
        if (pos->status != 3) {
            SetHighLight3(imgCur, &pageRender, pos->s_contentID, pos->s_offset, pos->e_contentID,
                          pos->e_offset, m_vColor[pos->colorIndex]);

        }
    }
//	m_SZImageCurrent.writeBMPFile( ("c:\\SZDispOut.bmp"));
    return true;
}

bool SZEbookReader::SetFont(const char *fontName, const char *fontPath, bool IsChineseFont) {
    UpdateCurNoteToBookNote();
    if (IsChineseFont) {
        strcpy(m_strfontname, fontName);
        strcpy(m_strfontFilePath, fontPath);
        return fontManger.Init(m_strfontFilePath);
    } else {
        strcpy(m_strEfontname, fontName);
        strcpy(m_strEfontFilePath, fontPath);
        return E_fontManger.Init(m_strEfontFilePath);
    }
}

bool SZEbookReader::SetChineseFont(const char *fontName, const char *fontPath) {
    return SetFont(fontName, fontPath, true);
}

bool SZEbookReader::SetEnglishFont(const char *fontName, const char *fontPath) {
    return SetFont(fontName, fontPath, false);
}

void SZEbookReader::ClearCurPageWordInfo() {
    int i;
    Vt_lineinfo vli = m_curPageWordInfo.lines;

    for (i = 0; i < m_curPageWordInfo.lines.size(); i++) {
        m_curPageWordInfo.lines[i].words.clear();
    }
    m_curPageWordInfo.lines.clear();
    m_curPageWordInfo.images.clear();
}

curPageNotes *SZEbookReader::GetCurPageNotes() {
    return &m_curPageNotes;
}

void SZEbookReader::SetHighLightColorType(ColorType type) {
    m_HighLightColorType = type;
}

int SZEbookReader::GetTabSize() {
    return m_myRender.GetTabSize();
}

void SZEbookReader::SetTabSize(int val) {
    m_myRender.SetTabSize(val);
}

int SZEbookReader::GetVertSpac() {
    return m_myRender.GetVertSpac();
}

void SZEbookReader::SetVertSpac(float val) {
    m_myRender.SetVertSpac(val);
    m_IsAutoCalcVert = false;
}

int SZEbookReader::GetParagraphSpac() {
    return m_myRender.GetParagraphSpac();
}

void SZEbookReader::SetParagraphSpac(float val) {
    m_myRender.SetParagraphSpac(val);
}

int SZEbookReader::GetWordSpac() {
    return m_myRender.GetWordSpac();
}

void SZEbookReader::SetWordSpac(int val) {
    m_myRender.SetWordSpac(val);
}

int SZEbookReader::GetMrginLeft() {
    return m_myRender.GetMrginLeft();
}

void SZEbookReader::SetMrginLeft(int val) {
    m_myRender.SetMrginLeft(val);
}

int SZEbookReader::GetMrginRight() {
    return m_myRender.GetMrginRight();
}

void SZEbookReader::SetMrginRight(int val) {
    m_myRender.SetMrginRight(val);
}

int SZEbookReader::GetMrginTop() {
    return m_myRender.GetMrginTop();
}

void SZEbookReader::SetMrginTop(int val) {
    m_myRender.SetMrginTop(val);
}

int SZEbookReader::GetMrginBottom() {
    return m_myRender.GetMrginBottom();
}

void SZEbookReader::SetMrginBottom(int val) {
    m_myRender.SetMrginBottom(val);
}

Vt_curPageMultiMedia SZEbookReader::GetCurPageMultiMedia() {
    return m_curPageMultimedia;
}

void *SZEbookReader::GotoInterPage(string url, SZImageBit24 &imageCur, long &imageSize,
                                   t_PageOutData &pageOutData) {
    int page = -1;
    int i;
    vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
    Record infor = bookreader.m_book->GetChapterInforFromFilePath(url);
    myBookRecord record;

    for (i = 0; i < vfile.size(); i++) {
        string f = vfile[i];
        if (vfile[i] == infor.file) {
            record.vfileID = i;
            record.contentNumber = infor.contentNumber;//0;//
            record.offer = infor.offer;
            return GetPageByRecord(record, imageCur, imageSize, pageOutData, true);
        }
    }
    return NULL;
}

void SZEbookReader::SetEncodingType(int encodtype) {
    if (encodtype == 1 || encodtype == 2 || encodtype == 0) {
        m_curtxtfileencodetype = encodtype;
    }
}

int SZEbookReader::GetCurFileEncodeType() {
    return m_curtxtfileencodetype;
}

map<int, Vt_myBookRecord> SZEbookReader::GetEbookPageInfo() {
    //test
    // 	map<int,Vt_myBookRecord>::iterator pos;
    // 	for (pos = m_mapAllPageInfo.begin();pos != m_mapAllPageInfo.end();pos++)
    // 	{
    // 		int fileid = pos->first;
    // 		Vt_myBookRecord vRecord = pos->second;
    // 		for (int  i = 0; i< vRecord.size();i++)
    // 		{
    // 			myBookRecord temp = vRecord[i];
    // 			TRACE("fileid %d  vfileid:%d contentnumber:%d offset:%d\n",fileid,temp.vfileID,temp.contentNumber,temp.offer);
    // 		}
    // 	}
    return m_mapAllPageInfo;
}

void SZEbookReader::SetEbookPageInfo(map<int, Vt_myBookRecord> mappageinfo) {
    LOGI("SZEbookReader::SetEbookPageInfo| begin");
    m_mapAllPageInfo.clear();
    m_mapAllPageInfo = mappageinfo;
    LOGI("SZEbookReader::SetEbookPageInfo| end");
}

bookinfo SZEbookReader::GetBookInfo(const char *file, int coverWidth, int coverHeight) {
    //	_CrtSetBreakAlloc(2341);

    char *ptr = strrchr((char *) file, '.');
    if (ptr) {
        ptr++;
        if (myStrCaseCmp(ptr, "txt") == 0) {
            m_curFileType = FILETYPE_TXT;
        }
    }

    bookinfo temp;
    if (m_curFileType == FILETYPE_TXT) {
        return temp;
    }
    if (bookreader.OpenFile((char *) file) != 0) {
        m_status = -1;
        return temp;
    }
    int length;
    myWcharT *wchar = bookreader.m_book->m_bookInfo->GetCreator();
    if (wchar != NULL) {
        length = myWcslen(wchar);
        temp.author = new myWcharT[length + 1];
        if (temp.author == NULL) {
            return temp;
        }
        memset(temp.author, 0, (length + 1) * sizeof(myWcharT));
        memcpy(temp.author, wchar, length * sizeof(myWcharT));
    }

    wchar = bookreader.m_book->m_bookInfo->GetTitle();
    if (wchar != NULL) {
        length = myWcslen(wchar);
        temp.title = new myWcharT[length + 1];
        if (temp.title == NULL) {
            return temp;
        }
        memset(temp.title, 0, (length + 1) * sizeof(myWcharT));
        memcpy(temp.title, wchar, length * sizeof(myWcharT));
    }

    wchar = bookreader.m_book->m_bookInfo->GetPublisher();
    if (wchar != NULL) {
        length = myWcslen(wchar);
        temp.publisher = new myWcharT[length + 1];
        if (temp.publisher == NULL) {
            return temp;
        }
        memset(temp.publisher, 0, (length + 1) * sizeof(myWcharT));
        memcpy(temp.publisher, wchar, length * sizeof(myWcharT));
    }

    wchar = bookreader.m_book->m_bookInfo->Getdate();
    if (wchar != NULL) {
        length = myWcslen(wchar);
        temp.date = new myWcharT[length + 1];
        if (temp.date == NULL) {
            return temp;
        }
        memset(temp.date, 0, (length + 1) * sizeof(myWcharT));
        memcpy(temp.date, wchar, length * sizeof(myWcharT));
    }

    wchar = bookreader.m_book->m_bookInfo->GetSubject();
    if (wchar != NULL) {
        length = myWcslen(wchar);
        temp.subject = new myWcharT[length + 1];
        if (temp.subject == NULL) {
            return temp;
        }
        memset(temp.subject, 0, (length + 1) * sizeof(myWcharT));
        memcpy(temp.subject, wchar, length * sizeof(myWcharT));
    }

    wchar = bookreader.m_book->m_bookInfo->Getdescription();
    if (wchar != NULL) {
        length = myWcslen(wchar);
        temp.description = new myWcharT[length + 1];
        if (temp.description == NULL) {
            return temp;
        }
        memset(temp.description, 0, (length + 1) * sizeof(myWcharT));
        memcpy(temp.description, wchar, length * sizeof(myWcharT));
    }

    wchar = bookreader.m_book->m_bookInfo->Getlanguage();
    if (wchar != NULL) {
        length = myWcslen(wchar);
        temp.language = new myWcharT[length + 1];
        if (temp.language == NULL) {
            return temp;
        }
        memset(temp.language, 0, (length + 1) * sizeof(myWcharT));
        memcpy(temp.language, wchar, length * sizeof(myWcharT));
    }

    long coversize = 0;
    temp.cover = (myBYTE *) GetCoverImg(coversize, coverWidth, coverHeight, true);
    if (temp.cover) {
        temp.coverDatalength = coversize;
    }
    bookreader.Close();
    return temp;
}


void SZEbookReader::SetCurrentPageNumber(int number) {
    if (number > 0) {
        m_CurrentPageNum = number;
    }

}

void SZEbookReader::SetCurrentBookRecordFileDir(const char *fileDir) {
    if (fileDir != NULL) {
        if (strlen(fileDir) != 0) {
            m_currentPageRecordFileDir.assign(fileDir);
#ifdef _WIN32
                                                                                                                                    if (fileDir[strlen(fileDir)-1] != '\\')
            {
                m_currentPageRecordFileDir.append("\\");
            }
#else
            if (fileDir[strlen(fileDir) - 1] != '/') {
                m_currentPageRecordFileDir.append("/");
            }
#endif
        }

    }
}

//��ͼ��֮�󣬸��ݼ�¼��  ��Ҫisrender = false
void *SZEbookReader::GetPageByRecord(myBookRecord record, SZImageBit24 &imageCur, long &imageSize,
                                     t_PageOutData &pageOutData, bool isRender,
                                     bool isNeedDrawPage) {


    pageOutData.pwi.lines.clear();
    pageOutData.pwi.images.clear();
    pageOutData.pmm.clear();
    vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
    if (record.vfileID >= vfile.size()) {
        return NULL;
    }
    //û������ҳ��
    if (!m_isCalcFinish) {
        int re;
        if (m_curFileType == FILETYPE_TXT) {
            m_currentvcontent = bookreader.m_book->m_vContent;
        } else {
            if (m_currenthtml != vfile[record.vfileID] ||
                ((m_currenthtml == vfile[record.vfileID]) && m_vtCurrntFileInfo.size() == 0)) {
                m_currenthtml = vfile[record.vfileID];
                re = calcCurrentHtml(record.vfileID, vfile[record.vfileID], isRender, false);
            } else
                re = 1;
            if (re > 0) {
                int vtFileInfo = m_vtCurrntFileInfo.size();
                for (int i = 0; i < vtFileInfo; i++) {
                    myBookRecord temp = m_vtCurrntFileInfo[i];
                    if ((m_vtCurrntFileInfo[i].contentNumber == record.contentNumber) &&
                        abs(m_vtCurrntFileInfo[i].offer - record.offer) <= 1) {
                        m_currentRecord = m_vtCurrntFileInfo[i];
                        m_smallCurrentPageNum = i;
                        break;
                    }

                    if (i < (vtFileInfo - 1)) {
                        if (record > m_vtCurrntFileInfo[i] && record < m_vtCurrntFileInfo[i + 1]) {
                            m_currentRecord = m_vtCurrntFileInfo[i];
                            m_smallCurrentPageNum = i;
                            break;
                        }
                    } else {
                        m_currentRecord = m_vtCurrntFileInfo[i];
                        m_smallCurrentPageNum = i;
                        break;
                    }
                }
            }
        }
        if (!isRender) {
            m_currentRecord = record;
        }
        return isNeedDrawPage ? DrawCurrentPage(imageCur, imageSize, pageOutData, &m_currentRecord)
                              : NULL;
    } else {
        //�Ѿ���������
        /*if (m_curFileType == FILETYPE_EPUB)
		{
			if (IsCalcFileVContent(record.vfileID))
			{
				LOCK_MUTEX(&m_LockBufferVContent);
				m_currentvcontent = m_mBufferVContent[record.vfileID];
				UNLOCK_MUTEX(&m_LockBufferVContent);
			}
			else
			{
                int ret = bookreader.ReadFileByFilePath(vfile[record.vfileID]);
				if (ret == OPENZIPFILE_DATA_ERROR)
				{
					m_status=-95;
					return 	NULL;
				}
                else if(ret == OPENZIPFILE_NOT_EXIST)
                {
                    bookreader.m_book->m_bookSpine->DeleteItem(record.vfileID);
                    vfile=bookreader.m_book->m_bookSpine->GetVSpine();

                }
				else
				{
					PushVcontentIntoBuffer(record.vfileID,bookreader.m_book->m_vContent);
					m_currentvcontent = bookreader.m_book->m_vContent;
				}

			}
		}
		else
			m_currentvcontent = bookreader.m_book->m_vContent;
		if (m_currentvcontent.size()<=0)
		{
			m_status=-96;
			return 	NULL;
		}	*/
        int page = 0;
        map<int, Vt_myBookRecord>::iterator itor;
        for (itor = m_mapAllPageInfo.begin(); itor != m_mapAllPageInfo.end(); itor++) {
            if (itor->first == record.vfileID) {
                m_vtCurrntFileInfo = itor->second;
                int vtFileInfo = m_vtCurrntFileInfo.size();
                if (vtFileInfo <= 0) {
                    m_status = -97;
                    return NULL;
                }

                for (int i = 0; i < vtFileInfo; i++) {
                    page += 1;
                    myBookRecord temp = m_vtCurrntFileInfo[i];
                    if ((temp.contentNumber == record.contentNumber) &&
                        abs(temp.offer - record.offer) <= 1) {
                        m_currentRecord = temp;
                        m_smallCurrentPageNum = i;
                        break;
                    }

                    if (i < (vtFileInfo - 1)) {
                        if (record > m_vtCurrntFileInfo[i] && record < m_vtCurrntFileInfo[i + 1]) {
                            m_currentRecord = m_vtCurrntFileInfo[i];
                            m_smallCurrentPageNum = i;
                            break;
                        }
                    } else {
                        m_currentRecord = m_vtCurrntFileInfo[i];
                        m_smallCurrentPageNum = i;
                        break;
                    }
                }
                m_CurrentPageNum = page;

                return isNeedDrawPage ? DrawCurrentPage(imageCur, imageSize, pageOutData,
                                                        &m_currentRecord) : NULL;
            } else {
                page += itor->second.size();
            }
        }
    }
    return NULL;
}

int SZEbookReader::GetPageNumberByRecord(myBookRecord record) {
    int pageNumber(0);
    if (m_isCalcFinish) {
        int page = 0;

        //delete ԭ���Ѿ��������ˣ����������¼�����
        /*vector<string>		vfile=bookreader.m_book->m_bookSpine->GetVSpine();
		string filepath = vfile[record.vfileID];
		if (m_curFileType == FILETYPE_EPUB)
		{
			if (IsCalcFileVContent(record.vfileID))
			{
				LOCK_MUTEX(&m_LockBufferVContent);
				m_currentvcontent = m_mBufferVContent[record.vfileID];
				UNLOCK_MUTEX(&m_LockBufferVContent);
			}
			else
				bookreader.OpenFileByFilePath(filepath);
		}
		*/
        map<int, Vt_myBookRecord>::iterator itor;
        Vt_myBookRecord vtCurrntFileInfo;
        for (itor = m_mapAllPageInfo.begin(); itor != m_mapAllPageInfo.end(); itor++) {
            if (itor->first == record.vfileID) {
                vtCurrntFileInfo = itor->second;
                int vtFileInfo = vtCurrntFileInfo.size();
                if (vtFileInfo <= 0) {
                    return 0;
                }

                for (int i = 0; i < vtFileInfo; i++) {
                    page += 1;
                    myBookRecord temp = vtCurrntFileInfo[i];
                    if ((temp.contentNumber == record.contentNumber) &&
                        abs(temp.offer - record.offer) <= 1) {
                        break;
                    }
                    if (i < (vtFileInfo - 1)) {
                        if (record > vtCurrntFileInfo[i] && record < vtCurrntFileInfo[i + 1]) {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                pageNumber = page;
                break;
            } else {
                page += itor->second.size();
            }
        }
    }
    return pageNumber;
}

int
SZEbookReader::digui(map<int, ChapterLevel> &orderchapter, map<int, ChapterLevel>::iterator &pos) {
    int startPageNumber(0), endPageNumber, returnPageNumber(1);
    map<int, ChapterLevel>::iterator nextPos = pos;
    int count(0);
    while (pos != orderchapter.end()) {
        if (m_outStop || m_stop) {
            return 0;
        }
        count++;
        nextPos++;
        if (nextPos != orderchapter.end()) {
            if (nextPos->second.level == pos->second.level) {//ֱ��push
                startPageNumber = PushPageRangeIntoMap(startPageNumber, pos, nextPos, 1);
            } else if (nextPos->second.level < pos->second.level) {//push�󷵻ؽ���ҳ
                return PushPageRangeIntoMap(startPageNumber, pos, nextPos, 1) - 1;
            } else {//�ݹ��ȥ�ȴ�����
                endPageNumber = digui(orderchapter, nextPos);

                vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
                Record infor = bookreader.m_book->GetChapterInforFromChapterOrder(pos->first);
                for (int i = 0; i < vfile.size(); i++) {
                    if (m_outStop || m_stop) {
                        return 0;
                    }
                    if (vfile[i] == infor.file) {
                        myBookRecord BR;
                        BR.vfileID = i;
                        BR.contentNumber = infor.contentNumber;
                        BR.offer = infor.offer;
                        /*	GetPageByRecord(BR,false);*/
                        startPageNumber = GetPageNumberByRecord(BR);
                        break;
                    }
                }
                pageNumberRange temp;
                temp.startNumber = startPageNumber;
                temp.endNumber = endPageNumber;
                m_ChapterOrderPageNumber.insert(std::pair<int, pageNumberRange>(pos->first, temp));

                if (nextPos == orderchapter.end()) {
                    return 1;
                }
                pos = nextPos;
                nextPos++;
                infor = bookreader.m_book->GetChapterInforFromChapterOrder(nextPos->first);
                for (int i = 0; i < vfile.size(); i++) {
                    if (m_outStop || m_stop) {
                        return 0;
                    }
                    if (vfile[i] == infor.file) {
                        myBookRecord BR;
                        BR.vfileID = i;
                        BR.contentNumber = infor.contentNumber;
                        BR.offer = infor.offer;
                        /*	GetPageByRecord(BR,false);*/
                        startPageNumber = GetPageNumberByRecord(BR);
                        break;
                    }
                }
            }
        } else {
            returnPageNumber = PushPageRangeIntoMap(0, pos, nextPos, NULL) - 1;
        }
        pos++;
    }
    return returnPageNumber;
}

int SZEbookReader::PushPageRangeIntoMap(int nStartPageNumber, map<int, ChapterLevel>::iterator pos,
                                        map<int, ChapterLevel>::iterator nextPos, int isPosNull) {
    //int startPageNumber,endPageNumber;
    int order, nextorder;
    Record infor, nextinfor;
    pageNumberRange temp;
    temp.startNumber = temp.endNumber = 0;
    vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
    order = pos->first;
    if (nStartPageNumber == 0) {
        infor = bookreader.m_book->GetChapterInforFromChapterOrder(order);
    }
    //have nextpos
    if (isPosNull != NULL) {
        nextorder = nextPos->first;
        nextinfor = bookreader.m_book->GetChapterInforFromChapterOrder(nextorder);
    }
    for (int i = 0; i < vfile.size(); i++) {
        if (m_outStop || m_stop) {
            return 0;
        }
        if (nStartPageNumber == 0) {
            if (vfile[i] == infor.file) {
                myBookRecord BR;
                BR.vfileID = i;
                BR.contentNumber = infor.contentNumber;
                BR.offer = infor.offer;
                /*GetPageByRecord(BR,false);*/
                temp.startNumber = GetPageNumberByRecord(BR);
            }
        } else
            temp.startNumber = nStartPageNumber;
        if (isPosNull != NULL) {
            if (vfile[i] == nextinfor.file) {
                myBookRecord BR;
                BR.vfileID = i;
                BR.contentNumber = nextinfor.contentNumber;
                BR.offer = nextinfor.offer;
                //	GetPageByRecord(BR,false);
                int nendPagenumber = GetPageNumberByRecord(BR);
                if (temp.startNumber == nendPagenumber) {
                    temp.endNumber = nendPagenumber;
                } else
                    temp.endNumber = nendPagenumber - 1;
                break;
            }
        } else {
            temp.endNumber = m_AllPageNum;
        }
        if (temp.startNumber != 0 && temp.endNumber != 0) {
            break;
        }
    }
    m_ChapterOrderPageNumber.insert(std::pair<int, pageNumberRange>(order, temp));
    return temp.endNumber + 1;
}

bool SZEbookReader::CalcMuciPageNumberRange() {
    if (!m_isNeedCalcMuciRange) {
        if (CbkFunction)
            CbkFunction(Cbkhost, "", CALLBACK_OUTLINE_PAGERANGE);
        return true;
    }
    m_ChapterOrderPageNumber.clear();

    int startPageNumber(-1)/*,endPageNumber*/;
    map<int, ChapterLevel> orderchapter = bookreader.m_book->m_bookChapter->GetMapOrderChapter();
    map<int, ChapterLevel>::iterator pos;
//	int tempCurPageNumber = m_CurrentPageNum;
    pos = orderchapter.begin();
    if (0 == digui(orderchapter, pos)) {
        if (m_stop) {
            if (CbkFunction && m_outStop)
                CbkFunction(Cbkhost, "calc stop", CALLBACK_STOP);
            return false;
        }
        return false;
    }
    //m_CurrentPageNum = tempCurPageNumber;
    SaveMuciRange();
    if (m_outStop) {
        CbkFunction(Cbkhost, "", CALLBACK_STOP);
        return true;
    }
    printf("calc muci range finish\n");
    if (CbkFunction)
        CbkFunction(Cbkhost, "", CALLBACK_OUTLINE_PAGERANGE);
    return true;
}

map<int, pageNumberRange> SZEbookReader::GetChapterPageNumberRange() {
    return m_ChapterOrderPageNumber;
}

chapterInfor SZEbookReader::GetChapterInfoByRecordAndChapterLevel(myBookRecord record, int level) {
    int pagenumber = GetPageNumberByRecord(record);
    return GetChapterInfoByPageNumber(pagenumber, level);
}

chapterInfor SZEbookReader::GetChapterInfoByPageNumber(int pageNumber, int level) {
    chapterInfor CI;
    CI.order = -1;
    if (level < 1) {
        return CI;
    }
    map<int, ChapterLevel> orderchapter = bookreader.m_book->m_bookChapter->GetMapOrderChapter();

    map<int, pageNumberRange>::iterator p;
    for (p = m_ChapterOrderPageNumber.begin(); p != m_ChapterOrderPageNumber.end(); p++) {
        if (p->second.startNumber <= pageNumber && p->second.endNumber >= pageNumber) {
            map<int, ChapterLevel>::iterator finder = orderchapter.find(p->first);
            if (finder != orderchapter.end()) {
                CI.chapterName = finder->second.chapter;
                CI.NumberRange.startNumber = p->second.startNumber;
                CI.NumberRange.endNumber = p->second.endNumber;
                CI.order = p->first;
                if (finder->second.level == level) {
                    break;
                }
            }
        }
    }
    return CI;
}

myWcharT *SZEbookReader::GetContentByRecordAndLength(myBookRecord record, int length) {
    vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
    if (record.vfileID >= vfile.size()) {
        return NULL;
    }
    int fileid = record.vfileID;
    vector<Content *> vContent;
    if (m_curFileType == FILETYPE_EPUB) {
        if (IsCalcFileVContent(fileid)) {
            LOCK_MUTEX(&m_LockBufferVContent);
            vContent = m_mBufferVContent[fileid];
            UNLOCK_MUTEX(&m_LockBufferVContent);
        } else {
            int re = bookreader.ReadFileByFilePath(vfile[fileid]);
            if (re < 0) {
                m_status = -92;
                return NULL;
            } else {
                vContent = bookreader.m_book->m_vContent;
            }

        }
    } else
        vContent = bookreader.m_book->m_vContent;
    if (vContent.size() > 0) {
        if (m_contentText) {
            delete[] m_contentText;
            m_contentText = NULL;
        }
        m_contentText = new myWcharT[length + 1];
        if (m_contentText == NULL) {
            return NULL;
        }
        memset(m_contentText, 0, sizeof(myWcharT) * (length + 1));
        int count(0);
        for (int i = record.contentNumber; i < vContent.size(); i++) {
            Content *pContent = vContent[i];
            int indexj = i == record.contentNumber ? record.offer : 0;
            int len = myWcslen(pContent->text);
            for (int j = indexj; j < len; j++) {
                m_contentText[count++] = pContent->text[j];
                if (count == length)
                    break;
            }
            if (count == length)
                break;
        }
    }
    return m_contentText;
}

void SZEbookReader::SaveMuciRange() {
    char filePath[1024];
    memset(filePath, 0, 1024);
    char item[128];


    sprintf(filePath, "%s%dc%d %s %s %d %d %d %d %d %d %d %dmuci %d.%s",
            m_currentPageRecordFileDir.c_str(),
            m_OutDevW, m_OutDevH, m_strfontname, m_strEfontname, m_ZoomRatio, GetMrginBottom(),
            GetMrginLeft(),
            GetMrginRight(), GetMrginTop(), GetTabSize(), GetVertSpac(), GetWordSpac(), m_calcVer,
            FileType);
    FILE *mucifile = fopen(filePath, "w+");
    if (mucifile == NULL) {
        return;
    }
    map<int, ChapterLevel> orderchapter = bookreader.m_book->m_bookChapter->GetMapOrderChapter();
    map<int, pageNumberRange>::iterator p;
    for (p = m_ChapterOrderPageNumber.begin(); p != m_ChapterOrderPageNumber.end(); p++) {
        memset(item, 0, 128);
#ifdef _DEBUG
                                                                                                                                CStringConver sc;
		int len = myWcslen(orderchapter[p->first].chapter);
		char* anChapter = new char[len*2+1];
		memset(anChapter,0,len*2+1);
		sc.UnicodeBigToGbk(orderchapter[p->first].chapter,len*sizeof(myWcharT),anChapter,len*2);
		sprintf(item,"%d | %d %s | %d -%d\n",p->first,orderchapter[p->first].level,anChapter,p->second.startNumber,p->second.endNumber);
		delete[] anChapter;
#else
        sprintf(item, "%d %d %d\n", p->first, p->second.startNumber, p->second.endNumber);
#endif
        fwrite(item, 1, strlen(item), mucifile);

    }
    fclose(mucifile);
}

void SZEbookReader::ReadLastMuciPageRange() {
    char filePath[1024];
    memset(filePath, 0, 1024);
    sprintf(filePath, "%s%dc%d %s %s %d %d %d %d %d %d %d %dmuci %d.%s",
            m_currentPageRecordFileDir.c_str(),
            m_OutDevW, m_OutDevH, m_strfontname, m_strEfontname, m_ZoomRatio, GetMrginBottom(),
            GetMrginLeft(),
            GetMrginRight(), GetMrginTop(), GetTabSize(), GetVertSpac(), GetWordSpac(), m_calcVer,
            FileType);
    FILE *recordFile = fopen(filePath, "r");
    char buf[30];
    memset(buf, 0, 30);
    if (recordFile) {
        m_ChapterOrderPageNumber.clear();
        while (fgets(buf, 30, recordFile)) {
            int order;
            pageNumberRange pnr;
            char *p = strtok(buf, " ");
            if (p) {
                order = atoi(p);
                p = strtok(NULL, " ");
                if (p) {
                    pnr.startNumber = atoi(p);
                    p = strtok(NULL, " ");
                    if (p) {
                        pnr.endNumber = atoi(p);
                    }
                }
            }
            m_ChapterOrderPageNumber.insert(pair<int, pageNumberRange>(order, pnr));
        }
        m_isNeedCalcMuciRange = false;
        fclose(recordFile);
    } else
        m_isNeedCalcMuciRange = true;
}

void SZEbookReader::Stop() {
    m_outStop = true;
    m_stop = true;
    m_CalcRender.m_iStop = true;
}

int SZEbookReader::GetLastError() {
    return m_error;

}

void SZEbookReader::SetCurPageWordInfo(pageWordinfo &pwi) {
    if (pwi.fileID == m_curPageWordInfo.fileID) {
        if (m_curPageWordInfo.lines.size() > 0 && pwi.lines.size() > 0) {
            if (m_curPageWordInfo.lines[0].words.size() > 0 && pwi.lines[0].words.size() > 0) {
                wordinfo wi1 = m_curPageWordInfo.lines[0].words[0];
                wordinfo wi2 = pwi.lines[0].words[0];
                if (wi1.contentID == wi2.contentID && wi1.offset == wi2.offset) {
                    return;
                }

            }
        }
    }

    ClearCurPageWordInfo();
    //��ֵPWI
    m_curPageWordInfo = pwi;


    //booknote��ȡm_curpagenote
    m_curPageNotes.Notes.clear();
    m_curPageNotes.maxID = 0;
    for (int k = 0; k < m_bookNotes.size(); k++) {
        note temp = m_bookNotes[k];
        if (m_curPageWordInfo.fileID < temp.fileId) {
            break;
        }
        if (m_curPageWordInfo.fileID == temp.fileId) {
            for (int i = 0; i < m_curPageWordInfo.lines.size(); i++) {
                lineinfo line = m_curPageWordInfo.lines[i];
                for (int j = 0; j < line.words.size(); j++) {
                    wordinfo wi = line.words[j];
                    if (wi.contentID >= temp.s_contentID && wi.contentID <= temp.e_contentID &&
                        wi.offset >= temp.s_offset && wi.offset <= temp.e_offset) {
                        curPageNote curnote;
                        curnote.type = temp.type;
                        curnote.curPageID = m_curPageNotes.maxID++;
                        curnote.curBookId = temp.noteId;
                        curnote.status = 0;
                        curnote.s_contentID = temp.s_contentID;
                        curnote.s_offset = temp.s_offset;
                        curnote.e_contentID = temp.e_contentID;
                        curnote.e_offset = temp.e_offset;
                        curnote.colorIndex = temp.colorIndex;
                        m_curPageNotes.Notes.push_back(curnote);
                        m_curColorIndex = temp.colorIndex + 1;
                        goto nextNote;
                    }
                }
            }
        }
        nextNote:
        continue;
    }
}

void SZEbookReader::SetCurrentRecord(myBookRecord &record) {
    m_currentRecord = record;
}

myBookRecord SZEbookReader::GetCurrentRecord() {
    return m_currentRecord;
}

void SZEbookReader::PushRecordIntoBuffer(myBookRecord &record) {
    if (m_bufferDrawPage.empty()) {
        m_bufferDrawPage.push(record);
    } else {
        myBookRecord tempRecord = m_bufferDrawPage.top();
        if (tempRecord != record) {
            m_bufferDrawPage.push(record);
        }
    }
//	TRACE("buffer size:%d\n",m_bufferDrawPage.size());
}

void SZEbookReader::PopRecordBuffer() {
    if (!m_bufferDrawPage.empty()) {
        m_bufferDrawPage.pop();
    }
//	TRACE("buffer size:%d\n",m_bufferDrawPage.size());
}

void SZEbookReader::SetPageendRecord(myBookRecord &record) {
    m_beforCalcRecord = record;
}

myBookRecord SZEbookReader::GetPageendRecord() {
    return m_beforCalcRecord;
}

char *SZEbookReader::GetPageInfoFileName() {
    //add  ����ҳ����Ϣ�ļ���
    sprintf(m_pageInfoFileName, "%s%dc%d %s %s %d %d %d %d %d %d %d %d %d.%s",
            m_currentPageRecordFileDir.c_str(),
            m_OutDevW, m_OutDevH, m_strfontname, m_strEfontname, m_ZoomRatio, GetMrginBottom(),
            GetMrginLeft(),
            GetMrginRight(), GetMrginTop(), GetTabSize(), GetVertSpac(), GetWordSpac(), m_calcVer,
            FileType);
    return m_pageInfoFileName;
}

void SZEbookReader::SetScreenRatio(double ratio) {
    ScreenRatio = ratio;
}

int SZEbookReader::GetWordTotleFromContent(vector<Content *> vc) {
    int totle(0);
    for (int i = 0; i < vc.size(); i++) {
        Content *temp = vc[i];
        if (vc[i]->contentType & CT_Text && vc[i]->text != NULL) {
            totle += myWcslen(vc[i]->text);
        }

    }
    return totle;
}

int SZEbookReader::CalcBookWordTotle(CZEpubReader *epubreader2, vector<string> vfile) {
//	m_calcBufferVContent.clear();
    m_bookCalcCurWordNumber = 0;
    if (m_curFileType == FILETYPE_TXT) {
        return 1;
    }
    if (m_bookWordTotle > 0) {
        return 1;
    }
    m_bookWordTotle = 0;
    vector<Content *> vContent;
    for (int i = 0; i < vfile.size(); i++) {
        if (m_stop) {
            if (CbkFunction && m_outStop)
                CbkFunction(Cbkhost, "", CALLBACK_STOP);
            return 1;
        }

        if (IsCalcFileVContent(i)) {
            LOCK_MUTEX(&m_LockBufferVContent);
            vContent = m_mBufferVContent[i];
            UNLOCK_MUTEX(&m_LockBufferVContent);
        } else {
            int re = epubreader2->ReadFileByFilePath(vfile[i]);
            if (re == OPENZIPFILE_DATA_ERROR) {
                return 0;
            } else if (re == OPENZIPFILE_NOT_EXIST) {//�ļ�������
                continue;
            }
            PushVcontentIntoBuffer(i, epubreader2->m_book->m_vContent);
            vContent = epubreader2->m_book->m_vContent;
        }
        m_bookWordTotle += GetWordTotleFromContent(vContent);
    }
    return 1;
}

myBookRecord
SZEbookReader::ReturnRecordByAddstus(int index, int contentSize, int fileID, int addstus,
                                     int strsize) {
    myBookRecord temp;
    if (index == contentSize && (addstus == 0 || addstus == strsize)) {
        temp.vfileID = fileID + 1;
        temp.contentNumber = 0;
        temp.offer = 0;
    } else if (addstus == strsize/*myWcslen(m_currentvcontent[i-1]->text)*/) {
        temp.vfileID = fileID;
        temp.contentNumber = index + 1;
        temp.offer = 0;
    } else {
        temp.vfileID = fileID;
        temp.contentNumber = index;
        temp.offer = addstus;
    }
    return temp;
}

void SZEbookReader::SuspendCalcThread() {

#ifdef _WIN32
    //	SuspendThread((HANDLE*)m_calcpagehThread);
#else
    g_IsDrawing = true;
#endif // _win32
}

void SZEbookReader::ResumeCalcThread() {
#ifdef _WIN32
                                                                                                                            //	ResumeThread((HANDLE*)m_calcpagehThread);
	//TRACE("drawcurrentpage end\n");
#else
    g_IsDrawing = false;
    pthread_mutex_lock(&g_ClacpageMutex);
    pthread_cond_signal(&g_cond);
    pthread_mutex_unlock(&g_ClacpageMutex);
#endif // _win32
}

void SZEbookReader::SetHighLight2(SZImageBit24 &imgCur, CRender *tempRender, Vt_wordinfo &words,
                                  myRGBQUAD bgColor) {


    //���ֲ�ͬcontentid����Ϊÿ��content���Լ����ֺţ���ʽ
    SZFTFont *pChiFtFont = NULL;
    SZFTFont *pEngFtFont = NULL;
    if (words.size() > 0) {
        wordinfo wi = words[0];
        int k;
        int contentID = words[0].contentID;
        Vt_wordinfo itemHightlightWords;
        for (k = 0; k < words.size(); k++) {
            if (words[k].contentID > contentID) {
                //�õ����content����ʽ

                int fontsize = (words[k - 1].textStyle.fontSize + m_ZoomRatio) *
                               (m_ScreenType == NormalScreen ? 1 : ScreenRatio);
                if (fontsize < 6) {
                    fontsize = 6;
                }
                pChiFtFont = fontManger.CreateFont(fontsize);
                pEngFtFont = E_fontManger.CreateFont(fontsize);

                pChiFtFont->SetIsBold(words[k - 1].textStyle.Bold);
                pChiFtFont->SetIsItalic(words[k - 1].textStyle.isItalic);

                pEngFtFont->SetIsBold(words[k - 1].textStyle.Bold);
                pEngFtFont->SetIsItalic(words[k - 1].textStyle.isItalic);

                //���Ƶ�ǰ��������
                tempRender->DrawHighLight(imgCur, pChiFtFont, pEngFtFont, itemHightlightWords,
                                          bgColor);
                contentID = words[k].contentID;
                wi = words[k];
                itemHightlightWords.clear();
            }
            itemHightlightWords.push_back(words[k]);
        }

        //�õ����content����ʽ
        int fontsize = (words[k - 1].textStyle.fontSize + m_ZoomRatio) *
                       (m_ScreenType == NormalScreen ? 1 : ScreenRatio);
        if (fontsize < 6) {
            fontsize = 6;
        }
        pChiFtFont = fontManger.CreateFont(fontsize);
        pEngFtFont = E_fontManger.CreateFont(fontsize);

        pChiFtFont->SetIsBold(words[k - 1].textStyle.Bold);
        pChiFtFont->SetIsItalic(words[k - 1].textStyle.isItalic);

        pEngFtFont->SetIsBold(words[k - 1].textStyle.Bold);
        pEngFtFont->SetIsItalic(words[k - 1].textStyle.isItalic);

        //���Ƶ�ǰ��������
        tempRender->DrawHighLight(imgCur, pChiFtFont, pEngFtFont, itemHightlightWords, bgColor);
        itemHightlightWords.clear();

        words.clear();

    }
}

int SZEbookReader::SetHighLight3(SZImageBit24 &imgCur, CRender *tempRender, int startContentID,
                                 int startOffset,/*int endPageId,*/int endContentID, int endOffset,
                                 myRGBQUAD bgColor, int booknoteID /*= -1*/ ) {
    Vt_wordinfo highlightWords;
    bool push(false);
    for (int i = 0; i < m_curPageWordInfo.lines.size(); i++) {
        lineinfo li = m_curPageWordInfo.lines[i];
        for (int j = 0; j < li.words.size(); j++) {
            wordinfo wi = li.words[j];
            if (startContentID == endContentID) {
                if (wi.contentID == startContentID && wi.offset >= startOffset &&
                    wi.offset <= endOffset) {

                    highlightWords.push_back(wi);
                    push = true;
                }
                if (push && ((wi.contentID == startContentID && wi.offset > endOffset) ||
                             wi.contentID > endContentID)) {
                    goto drawHightlight;
                }
            } else {
                if (wi.contentID == startContentID) {
                    if (wi.offset >= startOffset) {
                        highlightWords.push_back(wi);
                        push = true;
                    }
                } else if (wi.contentID == endContentID) {
                    if (wi.offset <= endOffset) {
                        highlightWords.push_back(wi);
                        push = true;
                    }
                } else if (wi.contentID > startContentID && wi.contentID < endContentID) {
                    highlightWords.push_back(wi);
                    push = true;
                }
                if (push && (wi.contentID == endContentID && wi.offset > endOffset ||
                             wi.contentID > endContentID)) {
                    goto drawHightlight;
                }
            }

// 			if (wi.contentID >= startContentID && wi.contentID <= endContentID && wi.offset >= startOffset && wi.offset <= endOffset)
// 			{
// 				highlightWords.push_back(wi);
// 				push = true;
// 			}
            if (push && (wi.contentID > endContentID ||
                         (wi.contentID == endContentID && wi.offset > endOffset))) {
                goto drawHightlight;
            }
        }
    }
    drawHightlight:
    if (push) {
        SetHighLight2(imgCur, tempRender, highlightWords, bgColor);
        return 1;
    } else
        return 0;

}

bool SZEbookReader::InitPageBmpByWordInfo(SZImageBit24 &image, CRender *tempRender) {
    // ��imgcur��ֵ
    if (!image.Init(m_SZImageBackgroud.GetWidth(), m_SZImageBackgroud.GetHeight())) {
        return false;
    }
    image.SetBits(m_SZImageBackgroud.GetBits(), 0, m_SZImageBackgroud.GetSize());

    Vt_wordinfo normalWords;
    for (int i = 0; i < m_curPageWordInfo.lines.size(); i++) {
        lineinfo li = m_curPageWordInfo.lines[i];
        for (int j = 0; j < li.words.size(); j++) {
            wordinfo wi = li.words[j];
            normalWords.push_back(wi);
        }
    }
    if (normalWords.size() > 0) {
        myRGBQUAD temp;
        temp.rgbReserved = (myBYTE) 300;
        SetHighLight2(image, tempRender, normalWords, temp);
    }


    for (int i = 0; i < m_curPageWordInfo.images.size(); i++) {
        imageInfo ii = m_curPageWordInfo.images[i];
        CZLFile zlImgfile((bookreader.m_epubFile));
        if (zlImgfile.OpenFile(ii.filePath.c_str()) >= 0) {
            CReadImageFile readimage;
            SZImageBase *pimage = NULL;
            if (0 == readimage.ReadImage(&zlImgfile, pimage)) {
                int Width = pimage->GetWidth();
                int Height = pimage->GetHeight();

                if (Width > ii.width) {
                    if (!ResampleImg(pimage, ii.width, ii.height)) {
                        if (pimage) {
                            delete pimage;
                            pimage = NULL;
                        }
                        m_status = -38;
                        m_IsDrawing = false;
                        ResumeCalcThread();
                        return -1;
                    }
                }

                tempRender->DrawImage(image, pimage, ii.x, ii.y);
            }
        }
    }

    return true;
}

Vt_notes SZEbookReader::GetBookNotes() {
    return m_bookNotes;
}


void *
SZEbookReader::GetCoverImg(long &imageSize, int coverWidth, int coverHeight, bool isGetBookinfo) {
    if (m_curFileType == FILETYPE_EPUB) {
        if (!isGetBookinfo && m_status != 1) {//ͨ��getboookinfo���õģ�����Ҫ�ж�m_status
            return NULL;
        }
        myBYTE *imageData = NULL;
        string coverpath = bookreader.m_book->GetCoverFile();
        CZLFile zlImgfile((bookreader.m_epubFile));
        if (zlImgfile.OpenFile(coverpath.c_str()) >= 0) {
            CReadImageFile readimage;
            SZImageBase *pimage = NULL;
            if (0 == readimage.ReadImage(&zlImgfile, pimage)) {
                int height = pimage->GetHeight();
                int width = pimage->GetWidth();
                int newWidth = coverWidth == -1 ? 99 : coverWidth;
                //double t = (double)width/newWidth;
                int newheight = coverHeight == -1 ? 99 : coverHeight/*height / t*/;
                if (newWidth >= width || newheight >= height) {
                    imageSize = pimage->GetSize();
                    imageData = new myBYTE[imageSize];
                    if (imageData) {
                        memcpy(imageData, pimage->GetBMP(), imageSize);
                    }
                    return imageData;
                }
                if (pimage->GetBitCount() == 24) {
                    Bit24ImageScalingAlgorithm b24wsa;
                    if (!b24wsa.Init(pimage->GetBits(), width, height, newWidth, newheight, 1)) {
                        return NULL;
                    } else {
                        b24wsa.DoZoom(true);
                        int nNewImgWidth = b24wsa.GetDstWidth();
                        int nNewImgHeight = b24wsa.GetDstHeight();
                        //	img->Decode((BYTE*)b24wsa.GetBMP(),b24wsa.GetSize(),CXIMAGE_FORMAT_BMP);
                        int length = b24wsa.GetSize();
                        imageData = new myBYTE[length];
                        memcpy(imageData, b24wsa.GetBMP(), length * sizeof(myBYTE));
                        imageSize = length;

                    }
                } else if (pimage->GetBitCount() == 8) {
                    Bit8ImageScalingAlgorithm b8wsa;
                    if (!b8wsa.Init(pimage->GetBits(), width, height, newWidth, newheight, 1))
                        return NULL;
                    else {
                        b8wsa.DoZoom(false);
                        int nNewImgWidth = b8wsa.GetDstWidth();
                        int nNewImgHeight = b8wsa.GetDstHeight();
                        //	img->Decode((BYTE*)b24wsa.GetBMP(),b24wsa.GetSize(),CXIMAGE_FORMAT_BMP);
                        int length = b8wsa.GetSize();
                        imageData = new myBYTE[length];
                        memcpy(imageData, b8wsa.GetBMP(), length * sizeof(myBYTE));
                        imageSize = length;
                    }
                }
                if (pimage) {
                    delete pimage;
                    pimage = NULL;
                }
            }
        }
        return imageData;
    } else
        return NULL;
}

int SZEbookReader::GetFontRealSize(char *fontName) {
    char *p = strrchr(fontName, '-');
    if (p) {
        p++;
        return atoi(p);
    } else
        return 0;
}

void *
SZEbookReader::DrawCurrentPage(SZImageBit24 &imageCur, long &imageSize, t_PageOutData &pageOutData,
                               myBookRecord *drawRecord/* = NULL*/,
                               vector<Content *> *vcontent /*= NULL*/ ) {
    //MessageBoxA(NULL,"DrawCurrentPageEx begin","a",MB_OK);
    LOGI("SZEbookReader::DrawCurrentPage | begin");
    pageOutData.pwi.lines.clear();
    pageOutData.pwi.images.clear();
    pageOutData.pmm.clear();
    m_IsDrawing = true;
    //��ʼ������ͼ
    if (!imageCur.Init(m_SZImageBackgroud.GetWidth(), m_SZImageBackgroud.GetHeight())) {
        m_IsDrawing = false;
        ResumeCalcThread();
        return NULL;
    }
    imageCur.SetBits(m_SZImageBackgroud.GetBits(), 0, m_SZImageBackgroud.GetSize());
    myBookRecord bookrecord = (drawRecord == NULL ? m_currentRecord : *drawRecord);
    if (!m_iCanFlip) {
        if (m_bufferDrawPage.empty()) {
            m_bufferDrawPage.push(bookrecord);
        } else {
            myBookRecord tempRecord = m_bufferDrawPage.top();
            if (tempRecord != bookrecord) {
                m_bufferDrawPage.push(bookrecord);
            }
        }
    }

    LOGI("SZEbookReader::DrawCurrentPage | will get htmlcontent");
    vector<Content *> vTempContent;
    if (1 != GetHtmlContent(bookrecord.vfileID, vTempContent)) {
        return NULL;
    }
    LOGI("SZEbookReader::DrawCurrentPage | finish get htmlcontent");
    //���ɱ�ҳrender����
    CRender pageRender = m_myRender;
    pageRender.Init(m_OutDevW, m_OutDevH, m_nFormat, m_clrForeG);

    int fileId = bookrecord.vfileID;
    int contentId = bookrecord.contentNumber;
    int offset = bookrecord.offer;
    int contentCount = vTempContent.size(); //(vcontent==NULL?m_currentvcontent.size():vcontent->size());

    //��ǰҳ������ʼ��
    pageOutData.PageNotes.maxID = 0;
    pageOutData.PageNotes.Notes.clear();
    m_curColorIndex = 0;

    pageOutData.pwi.fileID = fileId;
    lineinfo t_li;
    t_li.x = 0;
    t_li.y = 0;
    t_li.Width = 0;
    t_li.Height = 0;
    pageOutData.pwi.lines.push_back(t_li);
    pageOutData.pwi.contentID = contentId;

    int i = 0;
    int endOffset = 0;
    for (i = contentId; i < contentCount; i++) {
        Content *content = vTempContent.at(
                i); //(vcontent==NULL?m_currentvcontent[i]:vcontent->at(i));
        endOffset = 0;
        if (content->contentType & CT_Text) {
            LOGI("SZEbookReader::DrawCurrentPage | will DrawContentText");
            int rt = DrawContentText(imageCur, content, i, pageRender, offset, endOffset,
                                     pageOutData);
            LOGI("SZEbookReader::DrawCurrentPage | finish DrawContentText");
            if (rt == 0) {//��ǰҳ�Ѿ�û�еط����Ի�����
                //i++;
                goto end;
            }
            offset = 0;
        } else if (content->contentType & CT_Image || content->contentType & CT_ImagePage) {
            LOGI("SZEbookReader::DrawCurrentPage | will DrawContentImage");
            int rt = DrawContentImage(imageCur, content, pageRender, offset, pageOutData);
            LOGI("SZEbookReader::DrawCurrentPage | finish DrawContentImage");
            if (rt == 0) {
                //i++;
                endOffset = -2;
                goto end;
            } else if (rt == 2) {
                endOffset = -1;
                goto end;
            }
            offset = 0;
        }
//        else if (content->contentType & CT_ImagePage)
//        {
//            int rt = DrawContentImage(imageCur,content,pageRender,offset,pageOutData);
//            if (rt  == 0  ||rt  == 2)
//            {
//                //i++;
//                endOffset = -2;
//                goto end;
//            }
//            offset = -2;
//        }
    }

    end:
    //MessageBoxA(NULL,"DrawCurrentPageEx end","a",MB_OK);
    LOGI("SZEbookReader::DrawCurrentPage  drawContent finish");

    if (pageOutData.pwi.lines[pageOutData.pwi.lines.size() - 1].words.size() == 0) {
        pageOutData.pwi.lines.erase(pageOutData.pwi.lines.end() - 1);
    }

    //����beforRecord
    if (i == contentCount) {
        m_beforCalcRecord.vfileID = pageOutData.nextPageRecord.vfileID = bookrecord.vfileID + 1;
        m_beforCalcRecord.contentNumber = pageOutData.nextPageRecord.contentNumber = 0;
        m_beforCalcRecord.offer = pageOutData.nextPageRecord.offer = 0;
    } else if (i == contentCount - 1 && endOffset == -2) {
        m_beforCalcRecord.vfileID = pageOutData.nextPageRecord.vfileID = bookrecord.vfileID + 1;
        m_beforCalcRecord.contentNumber = pageOutData.nextPageRecord.contentNumber = 0;
        m_beforCalcRecord.offer = pageOutData.nextPageRecord.offer = 0;
    } else if (endOffset == -2) {
        m_beforCalcRecord.vfileID = pageOutData.nextPageRecord.vfileID = bookrecord.vfileID;
        m_beforCalcRecord.contentNumber = pageOutData.nextPageRecord.contentNumber = i + 1;
        m_beforCalcRecord.offer = pageOutData.nextPageRecord.offer = 0;
    } else {
        m_beforCalcRecord.vfileID = pageOutData.nextPageRecord.vfileID = bookrecord.vfileID;
        m_beforCalcRecord.contentNumber = pageOutData.nextPageRecord.contentNumber = i;
        m_beforCalcRecord.offer = pageOutData.nextPageRecord.offer = endOffset + 1;
    }

    //������
    int curPageNoteID = 0;
    for (int j = 0; j < m_bookNotes.size(); j++) {
        note temp = m_bookNotes[j];
        //pageidÊÇË³ÐòÅÅÁÐµÄ¡£³¬¹ýÁËŸÍÍ£Ö¹
        if (bookrecord.vfileID < temp.fileId) {
            break;
        }
        if (bookrecord.vfileID == temp.fileId) {
            if (SetHighLight3(imageCur, &pageRender, temp.s_contentID, temp.s_offset,
                              temp.e_contentID, temp.e_offset, m_vColor[temp.colorIndex],
                              temp.noteId)) {
                curPageNote curnote;
                curnote.type = temp.type;
                curnote.curPageID = curPageNoteID++;
                curnote.curBookId = temp.noteId;
                curnote.status = 0;
                curnote.s_contentID = temp.s_contentID;
                curnote.s_offset = temp.s_offset;
                curnote.e_contentID = temp.e_contentID;
                curnote.e_offset = temp.e_offset;
                curnote.colorIndex = temp.colorIndex;
                pageOutData.PageNotes.Notes.push_back(curnote);
                m_curColorIndex = temp.colorIndex + 1;
            }
        }
    }
    m_IsDrawing = false;
    ProcessPageWordInfo(pageOutData.pwi);
#ifdef _USE_BMP
                                                                                                                            imageSize = imageCur.GetSize();
	return imageCur.GetBMP();
#else
    CxImage tmp((BYTE *) imageCur.GetBMP(), imageCur.GetSize(), CXIMAGE_FORMAT_BMP);
    if (tmp.IsValid()) {
        BYTE *buf = NULL;
        tmp.Encode(buf, imageSize, CXIMAGE_FORMAT_PNG);
        return buf;
    } else
        return NULL;
#endif


}

int SZEbookReader::DrawContentText(SZImageBit24 &imageCur, Content *content, int contentId,
                                   CRender &tempRender, int offset, int &end,
                                   t_PageOutData &pageOutData) {
    SZFTFont *pChiFtFont = NULL;//�����������
    SZFTFont *pEngFtFont = NULL;//Ӣ���������

    if (content == NULL) {
        return -1;
    }
    int fontsize = (content->textStyle.fontSize + m_ZoomRatio) *
                   (m_ScreenType == NormalScreen ? 1 : ScreenRatio);
    if (fontsize < 6) {
        fontsize = 6;
    }
    pChiFtFont = fontManger.CreateFont(fontsize);
    pEngFtFont = E_fontManger.CreateFont(fontsize);
    if (pChiFtFont == NULL || pEngFtFont == NULL) {
        return -2;
    }

    int reCode = tempRender.DrawContentText(imageCur, pChiFtFont, pEngFtFont, contentId, content,
                                            offset, end, &pageOutData.pwi, &pageOutData.pmm);
    return reCode;


}

int SZEbookReader::DrawContentImage(SZImageBit24 &imageCur, Content *content, CRender &tempRender,
                                    int offset, t_PageOutData &pageOutData, bool isRender) {
/*ͼƬ���ù���
	1.���յȿ����ţ�
	2.����ȿ�֮�󣬸߶ȳ����ɻ�������߶ȣ���ô�Ѹ�ͼ�ŵ���һҳ
	3.�����һҳû�л��ƹ��֣���ô���յȸ��ٽ�������
*/

    //step1 �õ���ǰҳʣ����Ի��ƵĿ��

    int drawWidth = imageCur.GetWidth() - tempRender.GetMrginLeft() - tempRender.GetMrginRight();
    int drawHeight = imageCur.GetHeight() - tempRender.GetMrginBottom() - tempRender.GetOffsetY();

    bool NotDrawText = tempRender.GetOffsetY() == tempRender.GetMrginTop();//��ҳ�Ƿ���ƹ��֡�

    bool isImgPage = content->contentType & CT_ImagePage;

    MultimediaPage mp;
    mp.type = MT_Image;
    CZLFile zlImgfile((bookreader.m_epubFile));
    if (zlImgfile.OpenFile(content->filepath.c_str()) >= 0) {
        CReadImageFile readimage;
        SZImageBase *pimage = NULL;
        if (0 == readimage.ReadImage(&zlImgfile, pimage)) {
            int newWidth = pimage->GetWidth();
            int newHeight = pimage->GetHeight();

            if (!isRender) {
                mp.filePath = content->filepath;
// 				mp.dataSize = pimage->GetSize();
// 				mp.data = new myBYTE[mp.dataSize];
// 				if (mp.data)
// 				{
// 					memcpy(mp.data,pimage->GetBMP(),mp.dataSize);
// 				}
            }

            if (pimage->GetWidth() < drawWidth && pimage->GetHeight() < drawHeight) {

                if (isImgPage) {
                    float fx = (float) drawWidth / (float) pimage->GetWidth();
                    float fy = (float) drawHeight / (float) pimage->GetHeight();
                    if (fx < fy) {
                        newWidth = drawWidth;
                        newHeight = pimage->GetHeight() * fx;
                    } else {
                        newHeight = drawHeight;
                        newWidth = pimage->GetWidth() * fy;
                    }
                    SZImageBit24 *pDestImage = myResample(pimage, newWidth, newHeight);
                    if (pDestImage) {
                        delete pimage;
                        pimage = pDestImage;
                    }
                }

            } else {
                bool flag = false;
                if (pimage->GetWidth() > drawWidth) {
                    newWidth = drawWidth;
                    newHeight = (float) drawWidth / (float) pimage->GetWidth() *
                                (float) pimage->GetHeight();
                    if (newHeight > drawHeight) {
                        if (NotDrawText) {//�ٰ��յȸ�����
                            newWidth = (float) drawHeight / (float) pimage->GetHeight() *
                                       (float) pimage->GetWidth();
                            newHeight = drawHeight;
                        } else {
                            delete pimage;
                            return 2;
                        }
                    }
                    flag = true;
                } else if (pimage->GetHeight() > drawHeight) {
                    if (NotDrawText) {
                        newWidth = (float) drawHeight / (float) pimage->GetHeight() *
                                   (float) pimage->GetWidth();
                        newHeight = drawHeight;
                        flag = true;
                    } else {
                        delete pimage;
                        return 2;
                    }
                }
                if (flag) {
                    if (isRender) {//����ҳ�벻��Ҫ����������,�ѿ����Ϣ���õ�������
                        pimage->Destroy();
                        pimage->m_iWidth = newWidth;
                        pimage->m_iHeight = newHeight;
                    } else {
                        if (!ResampleImg(pimage, newWidth, newHeight)) {
                            if (pimage) {
                                delete pimage;
                                pimage = NULL;
                            }
                            m_status = -38;
                            m_IsDrawing = false;
                            ResumeCalcThread();
                            return -1;
                        }
                    }

                }
            }

            int rt = tempRender.DrawContentImage(imageCur, pimage, content, &mp, isRender);
            if (!isRender) {
                pageOutData.pmm.push_back(mp);

                imageInfo imgInfo;
                imgInfo.x = mp.lefttop_x;
                imgInfo.y = mp.lefttop_y;
                imgInfo.width = mp.rightbottom_x - mp.lefttop_x;
                imgInfo.height = mp.rightbottom_y - mp.lefttop_y;
                imgInfo.filePath = content->filepath;
                pageOutData.pwi.images.push_back(imgInfo);

            }
            delete pimage;
            pimage = NULL;
            return rt;

        } else {
            if (pimage) {
                delete[] pimage;
            }
            return -3;
        }
    }
    return -4;
}

int SZEbookReader::RenderContentEx(Vt_myBookRecord &vtCurrntFileInfo,
                                   vector<Content *> &currentvcontent, int fileID,
                                   bool isCallbackProgress/*=true*/ ) {

    m_CalcRender = m_myRender;
    int contentCount = currentvcontent.size();
    int i = 0;
    vtCurrntFileInfo.clear();

    myBookRecord tempRecord;
    tempRecord.vfileID = fileID;
    tempRecord.contentNumber = 0;
    tempRecord.offer = 0;
    vtCurrntFileInfo.push_back(tempRecord);

    SZImageBit24 imageCur;
    if (!imageCur.Init(m_SZImageBackgroud.GetWidth(), m_SZImageBackgroud.GetHeight())) {
        m_IsDrawing = false;
        ResumeCalcThread();
        return NULL;
    }


    for (i = 0; i < contentCount; i++) {
        if (m_stop) {
            return 1;
        }
        Content *content = currentvcontent[i];
        if (i == 20 && fileID == 4) {
            m_stop = false;
        }
        int contentOffset = 0;
        int endOffset = 0;
        if (content->contentType & CT_Text) {
            int rt(0);
            do {
                if (m_stop) {
                    return 1;
                }
                rt = RendContentText(content, m_CalcRender, contentOffset, endOffset);
                if (rt < 0) {//��������д�
                    return rt;
                }
                if (rt == 0) {
                    if (i == contentCount - 1 && endOffset == -2) {
                        goto CallBack;
                    } else if (endOffset == -2) {
                        tempRecord.vfileID = fileID;
                        tempRecord.contentNumber = i + 1;
                        tempRecord.offer = 0;
                    } else {
                        tempRecord.vfileID = fileID;
                        tempRecord.contentNumber = i;
                        tempRecord.offer = endOffset + 1;
                    }
                    vtCurrntFileInfo.push_back(tempRecord);
                }
                contentOffset = endOffset + 1;
            } while (endOffset != -2);


        } else if (content->contentType & CT_Image) {
            t_PageOutData temp;
            int rt = DrawContentImage(imageCur, content, m_CalcRender, 0, temp, true);
            if (rt < 0) {
                //return rt;
                continue;
            } else if (rt == 2) {//ͼƬ�Ų���
                tempRecord.vfileID = fileID;
                tempRecord.contentNumber = i;
                tempRecord.offer = 0;
                vtCurrntFileInfo.push_back(tempRecord);
                m_CalcRender.Init(m_OutDevW, m_OutDevH, m_nFormat, m_clrForeG);
                i--;

            } else if (rt == 0) {//��ͼƬ������֮�󣬲����ټ�������
                if (i == contentCount - 1) {
                    goto CallBack;
                } else {
                    tempRecord.vfileID = fileID;
                    tempRecord.contentNumber = i + 1;
                    tempRecord.offer = 0;
                }
                vtCurrntFileInfo.push_back(tempRecord);

            }
        } else if (content->contentType & CT_ImagePage) {
            m_CalcRender.Init(m_OutDevW, m_OutDevH, m_nFormat, m_clrForeG);
        }

        CallBack:        //�ص��������
        if (m_curFileType == FILETYPE_TXT) {
            float fstep = (float) i * 100 / contentCount;
            if (fstep >= m_fStep) {
                //�ص�����
                char iPro[10];
                sprintf(iPro, "%.2f", fstep);
                CbkFunction(Cbkhost, iPro, CALLBACK_CALC_PROGRESS);
                m_fStep += 1;
            }
        } else {
            if (m_isNeedCalcProcess && isCallbackProgress && m_bookWordTotle > 0 &&
                content->contentType & CT_Text && content->text != NULL) {
                m_bookCalcCurWordNumber += myWcslen(content->text);
                float fstep = (float) m_bookCalcCurWordNumber * 100 / m_bookWordTotle;
                if (fstep >= m_fStep) {
                    //�ص�����
                    char iPro[10];
                    sprintf(iPro, "%.2f", fstep);
                    CbkFunction(Cbkhost, iPro, CALLBACK_CALC_PROGRESS);
                    m_fStep += 1;
                }

            }
        }

    }
    return 1;
}

int SZEbookReader::RendContentText(Content *content, CRender &tempRender, int begin, int &end) {
    SZFTFont *pChiFtFont = NULL;//�����������
    SZFTFont *pEngFtFont = NULL;//Ӣ���������

    if (content == NULL) {
        return -1;
    }
    int fontsize = (content->textStyle.fontSize + m_ZoomRatio) *
                   (m_ScreenType == NormalScreen ? 1 : ScreenRatio);
    if (fontsize < 6) {
        fontsize = 6;
    }

    pChiFtFont = fontManger.CreateFont(fontsize);
    pEngFtFont = E_fontManger.CreateFont(fontsize);

    if (pChiFtFont == NULL || pEngFtFont == NULL) {
        return -2;
    }

    SZImageBit24 temp;
    pageWordinfo tpwi;
    Vt_curPageMultiMedia tCmm;
    int reCode = tempRender.DrawContentText(temp, pChiFtFont, pEngFtFont, 0, content, begin, end,
                                            &tpwi, &tCmm, true);

    ClearCurPageMultimedia(tCmm);
// 	delete pChiFtFont;
// 	pChiFtFont = NULL;
// 	delete pEngFtFont;
// 	pEngFtFont = NULL;
    return reCode;
}

void SZEbookReader::ClearCurPageMultimedia(Vt_curPageMultiMedia &vtPmm) {
// 	for (int i = 0; i < vtPmm.size(); i++)
// 	{
// 		if (vtPmm[i]->data != NULL)
// 		{
// 			delete[] vtPmm[i]->data;
// 			vtPmm[i]->data = NULL;
// 			vtPmm[i]->dataSize = 0;
// 		}
// 		delete vtPmm[i];
// 		vtPmm[i] = NULL;
// 	}
    vtPmm.clear();
}

bool SZEbookReader::GetMultiFileData(string filepath, void **data, long &size, MultimediaType mt) {
    bool re = false;
    CZLFile zlImgfile((bookreader.m_epubFile));
    if (zlImgfile.OpenFile(filepath.c_str()) >= 0) {
        if (mt == MT_Image) {
            CReadImageFile readimage;
            SZImageBase *pimage = NULL;
            if (0 == readimage.ReadImage(&zlImgfile, pimage)) {
                size = pimage->GetSize();
                *data = new unsigned char[size];
                if (*data) {
                    memcpy(*data, pimage->GetBMP(), size);
                    re = true;
                }
            }
        }

    }

    return re;
}

/*
return true - buffer���Ѿ�������
       false --buffer�в�����
*/
bool SZEbookReader::PushVcontentIntoBuffer(int fileId, vector<Content *> vContent) {
    LOCK_MUTEX(&m_LockBufferVContent);
    map<int, vector<Content *> >::iterator pos = m_mBufferVContent.find(fileId);
    if (pos == m_mBufferVContent.end()) {
        m_mBufferVContent.insert(pair<int, vector<Content *> >(fileId, vContent));
        UNLOCK_MUTEX(&m_LockBufferVContent);
        return false;
    } else {
        UNLOCK_MUTEX(&m_LockBufferVContent);
        return true;
    }
}

bool SZEbookReader::IsCalcFileVContent(int fileId) {
    LOCK_MUTEX(&m_LockBufferVContent);
    map<int, vector<Content *> >::iterator pos = m_mBufferVContent.find(fileId);
    UNLOCK_MUTEX(&m_LockBufferVContent);
    return pos != m_mBufferVContent.end();
}

void SZEbookReader::ClearBuferVContent() {
    LOCK_MUTEX(&m_LockBufferVContent);

    map<int, vector<Content *> >::iterator pos;
    for (pos = m_mBufferVContent.begin(); pos != m_mBufferVContent.end(); pos++) {
        vector<Content *> vContent = pos->second;
        for (int i = 0; i < vContent.size(); i++) {
            Content *con = vContent[i];
            if (con->after) {
                delete[] con->after;
                con->after = NULL;
            }
            if (con->before) {
                delete[] con->before;
                con->before = NULL;
            }
            if (m_curFileType == FILETYPE_TXT) {
                if (con->txt_SrcPtext) {
                    delete[] con->txt_SrcPtext;
                }
            } else {
                if (con->text) {
                    delete[] con->text;
                    con->text = NULL;
                }
            }
            delete con;
            con = NULL;
        }
        vContent.clear();
    }
    m_mBufferVContent.clear();
    UNLOCK_MUTEX(&m_LockBufferVContent);
}

void SZEbookReader::ProcessPageWordInfo(pageWordinfo &pwi) {
    for (int i = 0; i < pwi.lines.size(); i++) {
        int minx(1000000000), miny(100000000), maxHeight(0);
        for (int j = 0; j < pwi.lines[i].words.size(); j++) {
            if (pwi.lines[i].words[j].x < minx)
                minx = pwi.lines[i].words[j].x;

            if (pwi.lines[i].words[j].y < miny)
                miny = pwi.lines[i].words[j].y;
            if (pwi.lines[i].words[j].height > maxHeight) {
                maxHeight = pwi.lines[i].words[j].height;
            }

        }
        pwi.lines[i].x = minx;
        pwi.lines[i].y = miny;
        if (pwi.lines[i].words.size() > 0) {
            pwi.lines[i].Width = pwi.lines[i].words[pwi.lines[i].words.size() - 1].x +
                                 pwi.lines[i].words[pwi.lines[i].words.size() - 1].width -
                                 pwi.lines[i].x;
            pwi.lines[i].Height = maxHeight;
        } else {
            pwi.lines[i].Width = 0;
            pwi.lines[i].Height = maxHeight;
        }

    }

    if (m_ScreenType == HDScreen) {
        for (int i = 0; i < pwi.lines.size(); i++) {
            pwi.lines[i].x = (int) ((double) pwi.lines[i].x / ScreenRatio);
            pwi.lines[i].y = (int) ((double) pwi.lines[i].y / ScreenRatio);
            pwi.lines[i].Width = (int) ((double) pwi.lines[i].Width / ScreenRatio);
            pwi.lines[i].Height = (int) ((double) pwi.lines[i].Height / ScreenRatio);
            for (int j = 0; j < pwi.lines[i].words.size(); j++) {
                pwi.lines[i].words[j].x = (int) ((double) pwi.lines[i].words[j].x / ScreenRatio);
                pwi.lines[i].words[j].y = (int) ((double) pwi.lines[i].words[j].y / ScreenRatio);
                pwi.lines[i].words[j].width = (int) ((double) pwi.lines[i].words[j].width /
                                                     ScreenRatio);
                pwi.lines[i].words[j].height = (int) ((double) pwi.lines[i].words[j].height /
                                                      ScreenRatio);
            }
        }
    }
}

int SZEbookReader::calcHtml(int fileID) {
    vector<Content *> tempVContent;
    if (m_curFileType == FILETYPE_EPUB) {
        if (IsCalcFileVContent(fileID)) {
            LOCK_MUTEX(&m_LockBufferVContent);
            tempVContent = m_mBufferVContent[fileID];
            UNLOCK_MUTEX(&m_LockBufferVContent);
        } else {
            vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
            int re = bookreader.ReadFileByFilePath(vfile[fileID]);
            if (re == OPENZIPFILE_DATA_ERROR) {
                m_status = -72;
                return -1;
            } else if (re == OPENZIPFILE_NOT_EXIST) {
                return 3;
            }
            PushVcontentIntoBuffer(fileID, bookreader.m_book->m_vContent);
            tempVContent = bookreader.m_book->m_vContent;
        }

    } else {
        tempVContent = bookreader.m_book->m_vContent;
    }

    if (tempVContent.size() <= 0) {
        return 0;
    }

    m_vtCurrntFileInfo.clear();

    if (RenderContentEx(m_vtCurrntFileInfo, tempVContent, fileID, false) != 1) {
        m_bookCalcCurWordNumber = 0;
        m_status = -83;
        return false;
    }

    if (m_vtCurrntFileInfo.size() <= 0) {
        return 0;
    }
    return 1;
}

int SZEbookReader::GetCurrentPageNum() {
    if (m_iCanFlip) {
        return m_CurrentPageNum;
    } else
        return m_smallCurrentPageNum + 2;
}

int SZEbookReader::GetHtmlContent(int &htmlId, vector<Content *> &vContent) {

    LOGI("SZEbookReader::GetHtmlContent| begin | htmlid:%d", htmlId);
    vector<string> vfile = bookreader.m_book->m_bookSpine->GetVSpine();
    if (htmlId >= vfile.size()) {
        return -1;
    }
    if (m_curFileType == FILETYPE_EPUB) {
        while (1) {
            if (htmlId >= vfile.size()) {
                return -1;
            }
            m_currenthtml = vfile[htmlId];
            if (IsCalcFileVContent(htmlId)) {
                LOCK_MUTEX(&m_LockBufferVContent);
                vContent = m_mBufferVContent[htmlId];
                UNLOCK_MUTEX(&m_LockBufferVContent);
                break;
            } else {
                LOGI("SZEbookReader::GetHtmlContent| begin | will bookreader.ReadFileByFilePath m_currenthtml:%s",
                     m_currenthtml.c_str());
                int re = bookreader.ReadFileByFilePath(m_currenthtml);
                LOGI("SZEbookReader::GetHtmlContent| begin | re:%d", re);
                if (re == OPENZIPFILE_DATA_ERROR) {
                    return -2;
                } else if (re == OPENZIPFILE_NOT_EXIST) {
                    bookreader.m_book->m_bookSpine->DeleteItem(htmlId);
                    htmlId++;
                    //vfile=bookreader.m_book->m_bookSpine->GetVSpine();
                    continue;
                } else {
                    if (bookreader.m_book->m_vContent.size() <= 0) {
                        htmlId++;
                        continue;
                    }
                    PushVcontentIntoBuffer(htmlId, bookreader.m_book->m_vContent);
                    vContent = bookreader.m_book->m_vContent;
                    break;
                }

            }
        }

    } else
        vContent = bookreader.m_book->m_vContent;
    if (vContent.size() <= 0) {
        return -3;
    }
    return 1;
}

SZImageBit24 *SZEbookReader::myResample(SZImageBase *SrcBmp, int outWidth, int outHeight) {
    float xScale = 0;
    float yScale = 0;
    float xSrc = 0;
    float ySrc = 0;

    int inWidth = SrcBmp->GetWidth();
    int inHeight = SrcBmp->GetHeight();
    xScale = (float) inWidth / (float) outWidth;
    yScale = (float) inHeight / (float) outHeight;


    SZImageBit24 *pDestImage = new SZImageBit24();
    if (pDestImage) {
        pDestImage->Init(outWidth, outHeight);
        //nearest pix
        for (int y = 0; y < outHeight; y++) {
            ySrc = y * yScale;
            for (int x = 0; x < outWidth; x++) {
                xSrc = x * xScale;
                pDestImage->SetDesPixel(x, y, SrcBmp->GetOriPixel((long) xSrc, (long) ySrc), 255);
            }
        }

    }

    return pDestImage;

}
