#ifndef _BOOKCHAPTER_H
#define _BOOKCHAPTER_H
#include "myfDev/MYFDefine.h"

typedef struct _ChapterLevel
{
	myWcharT* chapter;//章节名
	int level;//章节级别
}ChapterLevel;

typedef vector<string> VS;
typedef vector<myWcharT*> VW;
typedef vector<int> VI;
typedef map<int,ChapterLevel> MIW;

class CBookChapter
{
public:
	CBookChapter(void);
	~CBookChapter(void);
	void PutInfo(int order,myWcharT* chapter,string file,int level);
	bool GetFileByOrder(int order,string& file);
	bool GetFileByChapter(myWcharT* chapter,string& file);
	bool GetChapterByOrder(int order,myWcharT*& chapter);
	int GetFirstOrder();
	VW GetVChapter();
	VS GetVFile();
	MIW GetMapOrderChapter();
private:
	VI vOrder;
	VW vChapter;
	VS vFile;
	MIW mOrderChapter;
	int min_order;
	int max_order;
public:
	int GetEndOrder(void);
	bool bHaveChapter(void);
	void Clear(void);
};

#endif