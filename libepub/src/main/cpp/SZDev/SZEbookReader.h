// SZEbookReader.h: interface for the SZEbookReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SZEBOOKREADER_H__A3EC8046_96CB_40EB_967C_45F5CD16592C__INCLUDED_)
#define AFX_SZEBOOKREADER_H__A3EC8046_96CB_40EB_967C_45F5CD16592C__INCLUDED_
#include "Render.h"
#include "ZlibFbreader.h" 
// #include <vector>
// #include <map>
// using namespace std;
typedef void (*CaclStatusCbk)(void *phost, char * shortinfo, long step);
 
#define NormalScreen 0
#define HDScreen     1

/*#define ScreenRatio  2.0*/

#define firstPage    1
#define endPage      2
#define othererror   3

#define CALLBACK_CALCING        0  //计算页码开始
#define CALLBACK_ALLFINISH      1  //所有页全计算完
#define CALLBACK_CURHTMLFINISH  2//计算完当前html
#define CALLBACK_OUTLINE_PAGERANGE  3//计算完目录页码范围
#define CALLBACK_STOP               4//调用stop
#define CALLBACK_CALC_PROGRESS      5//计算过程中返回进度


typedef vector<myBookRecord> Vt_myBookRecord;
typedef Vt_myBookRecord::iterator Vt_myBookRecordIterator;
typedef map<int,Vt_myBookRecord> Map_myBookPageInfo;
typedef Map_myBookPageInfo::iterator Map_myBookPageInfoIterator;

class SZEbookReader  
{
public:
	SZEbookReader();
	~SZEbookReader();
    void initWithopp();
	bool				Init(int iWidth, int iHeight,/*int zoom, */int screenType = NormalScreen);
	bool				IsInit(){return m_bInit;} 
	bool				DispOutChage(int iWidth, int iHeight);
	bool				SaveCurrentPage2Bmp(const char *BMPfilePathName);
	void*				RefreshPage(SZImageBit24 &imageCur,long& imageSize);
	long				GetSize() const;
	void*				GetBMP() const;	
	bool				calcPage();
	int                 GetZoomRatio();
	int					calcCurrentHtml(int fileID,string filePath,bool isNeedRender, bool isCallbackProgress = true);
	bool				calcOtherHtml();
	int					GetStatus(){return m_status;}

	bool				OpenEbook(const char *file);
	void				CloseEbook();
	void*				GetCoverImg(long& imageSize,int coverWidth = -1,int coverHeight = -1,bool isGetBookinfo = false);

	void*				DrawCurrentPage(SZImageBit24 &imageCur,long& imageSize,t_PageOutData& pageOutData,myBookRecord* drawRecord = NULL, vector<Content*>* vcontent = NULL);
	
	void*		 	    GotoPage(int PageNum,SZImageBit24 &imageCur,long& imageSize,t_PageOutData& pageOutData,int flag=0);
	void*		 	    NextPage(SZImageBit24 &imageCur,long& imageSize,t_PageOutData& pageOutData);
	void*		 	    PrevPage(SZImageBit24 &imageCur,long& imageSize, t_PageOutData& pageOutData);	

    unsigned int        GetFileCount();
    unsigned int        GetFilePageCount(unsigned int fileId);
    void*               GetPageByFileSmallPageNumber(unsigned int fileId, unsigned int smallPageNumber, SZImageBit24 &imageCur,long& imageSize,t_PageOutData& pageOutData);
    unsigned int        GetfileIdByPageNumber(unsigned int pageNumber);
    unsigned int        GetSmallPageNumberByPageNumber(unsigned int pageNumber);
    unsigned int        GetSmallPageNumberByRecord(myBookRecord record);
    myBookRecord        GetRecordByMuciIndex(int muciIndex);
    
	int	 			    GetAllPageNum()const{return m_AllPageNum;}
	int	 			    GetCurrentPageNum();
	void*				ReadCorrespondPageByMuciIndex(int MuciIndex,SZImageBit24 &imageCur, long& imageSize,t_PageOutData& pageOutData);
	myDWORD				GetFormat()const{return m_nFormat;}
	void				SetFormat(myDWORD val){m_nFormat = val;}

	bool				SetZoomRatio(int zoom );
	bool			    SetDefaultZoom();
	bool			    SetZoomBig();
	bool			    SetZoomSmall();
	void				PutBookNotes(Vt_notes booknotes);
	Vt_notes			GetBookNotes();
	int					GetFlipStaus(){return m_iCanFlip;}//-1 不能翻页 ，0 小翻页 1 大翻页
	void				SetCallBackFunction(void * phost,CaclStatusCbk cbk){Cbkhost= phost;CbkFunction = cbk;}	
 

	void				SetClrForeG(myRGBQUAD clr){m_clrForeG=clr;}	
	myRGBQUAD 			GetClrForeG()const{return m_clrForeG;}	
	void				SetClrBackG(myRGBQUAD clr);
	myRGBQUAD 			GetClrBackG()const{return m_clrBackG;}	

	bool				SetBgFilePath(const char *BMPfilePathName);	
	const char *		GetBgFilePath()const{return m_strBgFilePath;}
	bool				IsBGEabled()const{return m_bEableBG;}
	bool				EableBG(bool bEnable);
	
	pageWordinfo		GetCurPageWordInfo();
	void				SetCurPageWordInfo(pageWordinfo& pwi);
	void*				DrawCurPageHighLight(SZImageBit24& imgCur,curPageNote& cpn,long& imagesize);
	bool				DeleteCurPageNoteById(SZImageBit24& imgCur,int id);
	curPageNotes*			GetCurPageNotes();
	bool				ReCalcPage();

//  add by myf at 2012-10-23
	void				SetCurrentRecord(myBookRecord &record);
	myBookRecord        GetCurrentRecord();

	void				SetPageendRecord(myBookRecord& record);
	myBookRecord		GetPageendRecord();
// add by myf  at 2012-10-31
	char*				GetPageInfoFileName();

	//add by myf at 2012-11-14
	void                SetScreenRatio(double ratio);

	bool				GetMultiFileData(string filepath,void** data,long& size,MultimediaType mt = MT_Image);
private:
	int					GetFontRealSize(char* fontName);
	bool				InitPageBmpByWordInfo(SZImageBit24& image,CRender* tempRender);
	bool				UpdateCurNoteToBookNote();
	void				ReleaseSource();
	
	bool				RenderBG();
	bool				ResampleImg(SZImageBase*& image ,int newWidth,int newHeight);
	bool				MakeBackgroudImage(const char *BMPfilePathName);	
	bool				MakeBackgroudImage(myRGBQUAD clr);
	void				readLastCalcPageRecord();
	void				saveLastRecord(const char *file);

	int PushPageRangeIntoMap(int nStartPageNumber,map<int,ChapterLevel>::iterator pos,map<int,ChapterLevel>::iterator nextPos,int isPosNull);
	int digui(map<int,ChapterLevel>& orderchapter,map<int,ChapterLevel>::iterator& pos);
    void ReadLastMuciPageRange();
	void SaveMuciRange();
    
	int  GetWordTotleFromContent(vector<Content*> vc);
	int  CalcBookWordTotle(CZEpubReader* epubreader2,vector<string>	vfile);

	myBookRecord        ReturnRecordByAddstus(int index,int contentSize,int fileID,int addstus,int strsize);
	void	SuspendCalcThread();
	void    ResumeCalcThread();

	//new function
	int	RenderContentEx(Vt_myBookRecord     &vtCurrntFileInfo, vector<Content*>  &currentvcontent,int fileID,bool isCallbackProgress=true);
	int RendContentText(Content* content,CRender& tempRender,int begin,int& end);
//	int RendContentImage(Content* content,CRender& tempRender,Vt_myBookRecord &vtCurrntFileInfo);
	

	int DrawContentText(SZImageBit24 &imageCur,Content* content,int contentId,CRender& tempRender,int offset,int& end,t_PageOutData& pageOutData);
	int DrawContentImage(SZImageBit24 &imageCur,Content* content,CRender& tempRender,int offset,t_PageOutData& pageOutData,bool isRender = false);

	bool IsCalcFileVContent(int fileId);
	bool PushVcontentIntoBuffer(int fileId,vector<Content*> vContent);
	void ClearBuferVContent();
	void ProcessPageWordInfo(pageWordinfo& pwi);
	int	calcHtml(int fileID);

	int GetHtmlContent(int& htmlId, vector<Content* >& vContent);
    
    SZImageBit24 * myResample(SZImageBase *SrcBmp,int outWidth,int outHeight);
public:
	SZFontManager fontManger;
	SZFontManager E_fontManger;

	int			m_fontManagerMaxIndex;
	CZEpubReader  bookreader;

	bool		 m_stop;
	bool		 m_isReturn;
	bool		 m_isRetrunMuciRange;
    bool         m_outStop;
    
    bool         m_IsDrawing;
    
    int          m_error;
public:
	//add by myf at 2012 -10-23
	

	void    SetHighLight2(SZImageBit24& imgCur,CRender* tempRender,Vt_wordinfo& words,myRGBQUAD bgColor);
	int		SetHighLight3(SZImageBit24& imgCur,CRender* tempRender,int startContentID,int startOffset,/*int endPageId,*/int endContentID,int endOffset,myRGBQUAD bgColor,int booknoteID = -1);
	void	PopRecordBuffer();
	void	PushRecordIntoBuffer(myBookRecord& record);


    int     GetLastError();
    void    Stop();
    chapterInfor    GetChapterInfoByPageNumber(int pageNumber, int level);
	myWcharT*	GetContentByRecordAndLength(myBookRecord record,int length);//
	chapterInfor GetChapterInfoByRecordAndChapterLevel(myBookRecord record, int level);
    map<int,pageNumberRange> GetChapterPageNumberRange();
    int  GetPageNumberByRecord(myBookRecord record);
	void* GetPageByRecord(myBookRecord record, SZImageBit24 &imageCur,long& imageSize,t_PageOutData& pageOutData,bool isRender = false, bool isNeedDrawPage = true);
	void SetCurrentBookRecordFileDir(const char* fileDir);

	void SetHighLightColorType(ColorType type);
	void ClearCurPageWordInfo();
	void ClearCurPageMultimedia(Vt_curPageMultiMedia& vtPmm);
	bool SetFont(const char* fontName,const char* fontPath,bool IsChineseFont);
	bool SetChineseFont(const char* fontName,const char* fontPath);
	bool SetEnglishFont(const char* fontName,const char* fontPath);
	bool ModifyCurPageNoteById(SZImageBit24& imgCur,curPageNote& cpn);

    bool CalcMuciPageNumberRange();
	void SetCurrentPageNumber(int number);
	bookinfo GetBookInfo(const char *file,int coverWidth = -1,int coverHeight = -1);
	map<int,Vt_myBookRecord> GetEbookPageInfo();
	void SetEbookPageInfo(map<int,Vt_myBookRecord> mappageinfo);
	int GetCurFileEncodeType();
	void SetEncodingType(int encodtype);//1-utf8  2-gbk
	void* GotoInterPage(string url,SZImageBit24 &imageCur, long& imageSize,t_PageOutData& pageOutData);
	Vt_curPageMultiMedia GetCurPageMultiMedia();

	int			GetTabSize();
	void		SetTabSize(int val);
	
	int			GetVertSpac();
	void		SetVertSpac(float val);

	int			GetParagraphSpac();
	void		SetParagraphSpac(float val);
	
	int			GetWordSpac();
	void		SetWordSpac(int val);
	
	int		GetMrginLeft();
	void		SetMrginLeft(int val);
	
	int		GetMrginRight();
	void		SetMrginRight(int val);
	
	int		GetMrginTop();
	void		SetMrginTop(int val);
	
	int		GetMrginBottom();
	void		SetMrginBottom(int val); 
	

	//test
	//int     GetBufferSize();
private:
    
    bool m_isCalcFinish;
	bool m_isNeedCalcPage;
    bool			m_isNeedCalcMuciRange;

	int m_curtxtfileencodetype;
	int m_curFileType;
	pageWordinfo	m_curPageWordInfo;
	bool		m_IsAutoCalcVert;
	ColorType m_HighLightColorType;
	Vt_notes     m_bookNotes;//正本书的标注信息
	curPageNotes m_curPageNotes;//当前页到标注信息
	vector<myRGBQUAD> m_vColor;//标注颜色列表
	int			 m_curColorIndex;//当前标注颜色索引
	int			  m_OutDevW;
	int			  m_OutDevH;
	int			  m_ZoomRatio;
	int			  m_ZoomSpan;//字号直接的跨度
	int			  m_status;
    int           m_nVertSpac;
 
	myRGBQUAD	  m_clrBackG;
	myRGBQUAD	  m_clrForeG;
	myDWORD		  m_nFormat;
	bool		  m_bInit;
	bool		  m_bEableBG;
	bool          m_bIsBookOpen;
    
	int				   m_iCanFlip;
	int			       m_AllPageNum;
	int			       m_CurrentPageNum;

	vector<Content*>   m_currentvcontent;
	Vt_myBookRecord	   m_vtCurrntFileInfo;
	stack<myBookRecord> m_bufferDrawPage;
	Map_myBookPageInfo m_mapAllPageInfo;

	string             m_currenthtml;
	myBookRecord       m_currentRecord;
	myBookRecord       m_currentRecord2;
	myBookRecord       m_beforCalcRecord;
	int		   m_smallCurrentPageNum;
	myBookRecord	   m_lastBookRecord;
	bool			   m_bHaveRecord;

	void *        Cbkhost;   
    CaclStatusCbk CbkFunction;	

	SZImageBit24  m_SZImageBackgroud;
	SZImageBit24  m_SZImageCurrent;
	SZImageBit24  m_SZImageCashe;
	CRender				m_myRender;
	CRender			m_CalcRender;
	CRender			m_DrawRender;

	char		  m_strBgFilePath[256];
	char		  m_strEBookFilePath[256];

	char		  m_strfontname[256];
	char		  m_strfontFilePath[256];
	//char		  m_fontNamesNormal[30][256];
	char		  m_strEfontname[256];
	char		  m_strEfontFilePath[256];
	//char		  m_EfontNamesNormal[30][256];
	
	Vt_curPageMultiMedia m_curPageMultimedia;

	string			m_currentPageRecordFileDir;

	map<int,pageNumberRange>  m_ChapterOrderPageNumber;
	myWcharT* m_contentText;
    
    int m_ScreenType;
    bool m_isBuffPage;
	double ScreenRatio;
	//增加变量 返回页信息文件名
	char          m_pageInfoFileName[1024];
	map<int,vector<Content*> > m_mBufferVContent;//把通过ReadFileByFilePath计算出来的vcontent缓存起来 fileid->vcontent
	THREAD_MUTEX     m_LockBufferVContent;
    THREAD_MUTEX     m_lockAllPageInfo;
    THREAD_MUTEX     m_lockCalcCurHtml;

	long			m_bookWordTotle;
	long			m_bookCalcCurWordNumber;
	float			m_fStep;
	int				m_iOpenFileId;


	int				m_calcVer;
	THREAD_HANDLE   m_calcpagehThread;

	//add by 2013-5-6
	int				m_bookNotesMaxId;

	bool			m_isNeedCalcProcess;
};
#endif // !defined(AFX_SZEBOOKREADER_H__A3EC8046_96CB_40EB_967C_45F5CD16592C__INCLUDED_)
