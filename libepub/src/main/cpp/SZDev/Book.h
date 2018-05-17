#ifndef _BOOK_H
#define _BOOK_H

#include "HtmlParser.h"
#include "BookInfo.h"
#include "BookSpine.h"
#include "BookChapter.h"
#include "ZLFile.h"

typedef struct _Mark
{
	Record info;
	string markName;
}Mark;
typedef map<string,int> MSI;
class CBook
{
public:
	CBook(void* point);
	~CBook(void);
	CBookInfo* m_bookInfo;//图书信息()
	CBookSpine* m_bookSpine;//图书阅读顺序
	CBookChapter* m_bookChapter;//图书章节信息
	CZLFile* m_curFile;//当前打开的页面
	CZLFile* m_curCss;//当前已经解析出来的css文件
//	CZLFile* m_otherFile;//计算非当前页

	vector<Content* > m_vContent;
	int m_curFileEncodingtype;
	map<string,MSI> m_FIleAnchorContentNumber;
private:
	
	string m_coverFileName;//封面文件
	string m_bookname;//书名页面
	string m_curFileName;//当前打开的文件名
	void* m_unzFile;
	
public:
	int OpenFileByFilePath(string filepath);
	Record GetChapterInforFromFilePath(string filepath);
	void CopyMapAnchorContent(CBook* book);
	void PutCoverFile(string cover);
	void PutBookName(string& bookname);
	string GetCoverFile();
	string GetBookName();
	void ClearVContent( bool isTxt = false);

// 	void ResumeLastRecord();//从本地恢复上次阅读记录
// 	void ResumeLastMark();//从本机恢复书签记录
// 	vector<Mark> GetVMark();//获取全部书签信息
// 	Record GetLastRecord();//获取上次阅读记录信息

	void ProcessHtmlCSS(bool isHaveCSS);
	void PutAnchorContentNumber(map<string,int> anchorContent);

	map<string,int> GetCurFileAnchorContentNumber(void);
	int parser(string& filepath);

	Record GetChapterInforFromChapterOrder(int order);
};

#endif