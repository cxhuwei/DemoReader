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
	CBookInfo* m_bookInfo;//ͼ����Ϣ()
	CBookSpine* m_bookSpine;//ͼ���Ķ�˳��
	CBookChapter* m_bookChapter;//ͼ���½���Ϣ
	CZLFile* m_curFile;//��ǰ�򿪵�ҳ��
	CZLFile* m_curCss;//��ǰ�Ѿ�����������css�ļ�
//	CZLFile* m_otherFile;//����ǵ�ǰҳ

	vector<Content* > m_vContent;
	int m_curFileEncodingtype;
	map<string,MSI> m_FIleAnchorContentNumber;
private:
	
	string m_coverFileName;//�����ļ�
	string m_bookname;//����ҳ��
	string m_curFileName;//��ǰ�򿪵��ļ���
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

// 	void ResumeLastRecord();//�ӱ��ػָ��ϴ��Ķ���¼
// 	void ResumeLastMark();//�ӱ����ָ���ǩ��¼
// 	vector<Mark> GetVMark();//��ȡȫ����ǩ��Ϣ
// 	Record GetLastRecord();//��ȡ�ϴ��Ķ���¼��Ϣ

	void ProcessHtmlCSS(bool isHaveCSS);
	void PutAnchorContentNumber(map<string,int> anchorContent);

	map<string,int> GetCurFileAnchorContentNumber(void);
	int parser(string& filepath);

	Record GetChapterInforFromChapterOrder(int order);
};

#endif