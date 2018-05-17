#include "StdAfx.h"
#include "BookChapter.h"
#include "StringConver.h"
#include "MAlgorithm.h"
CBookChapter::CBookChapter(void)
{
	max_order = 0;
	min_order = 10;
}

CBookChapter::~CBookChapter(void)
{
	
	Clear();

}

void CBookChapter::PutInfo( int order,myWcharT* chapter,string file ,int level) 
{
	if (order > max_order)
	{
		max_order = order;
	}
	if (order < min_order)
	{
		min_order = order;
	}

	vOrder.push_back(order);
	vChapter.push_back(chapter);
	vFile.push_back(file);
	ChapterLevel chapterlevel;
	chapterlevel.chapter = chapter;
	chapterlevel.level = level;
	mOrderChapter.insert(pair<int,ChapterLevel>(order,chapterlevel));
	
}

bool CBookChapter::GetFileByOrder( int order,string& file )
{
	if (order > max_order || order < min_order)
	{
		return false;
	}
	int i;
	int ol = vOrder.size();
	for (i = 0;i < ol; i++)
	{
		if (vOrder[i] == order)
		{
			break;
		}
	}
	if (i < ol)
	{
		file = vFile[i];
		return true;
	}
	else
		return false;
}

bool CBookChapter::GetFileByChapter( myWcharT* chapter,string& file )
{
	int i;
	int oc = vChapter.size();
	for (i = 0;i < oc; i++)
	{
		if (myWcscmp(vChapter[i],chapter) == 0)
		{
			break;
		}
	}
	if (i < oc)
	{
		file = vFile[i];
		return true;
	}
	else
		return false;
}

bool CBookChapter::GetChapterByOrder( int order,myWcharT*& chapter )
{

	if (order > max_order || order < min_order)
	{
		return false;
	}
	int i;
	int ol = vOrder.size();
	for (i = 0;i < ol; i++)
	{
		if (vOrder[i] == order)
		{
			break;
		}
	}
	if (i < ol)
	{
		chapter = vChapter[i];
		return true;
	}
	else
		return false;
}

int CBookChapter::GetFirstOrder()
{
	return min_order;
}

VW CBookChapter::GetVChapter()
{
	VW vs;
	int i;
	for (i = min_order;i<= max_order;i++)
	{
		myWcharT* chapter;
		GetChapterByOrder(i,chapter);
		vs.push_back(chapter);
	}
	return vs;
}

VS CBookChapter::GetVFile()
{
	VS vs;
	int i;
	for (i = min_order;i<= max_order;i++)
	{
		string file;
		GetFileByOrder(i,file);
		vs.push_back(file);
	}
	return vs;
}

int CBookChapter::GetEndOrder(void)
{
	return max_order;
}


MIW CBookChapter::GetMapOrderChapter()
{
	return mOrderChapter;
}

bool CBookChapter::bHaveChapter(void)
{
	int ol = vOrder.size();
	if (ol>0)
	{
		return true;
	}
	return false;
}

void CBookChapter::Clear(void)
{
// 	for (unsigned int i = 0; i < vChapter.size(); i++)
// 	{
// 		if (vChapter[i] != NULL)
// 		{
// 			delete[] vChapter[i];
// 			vChapter[i] = NULL;
// 		}
// 	}
	vChapter.clear();
	vOrder.clear();
	vFile.clear();
	MIW::iterator pos;
	mOrderChapter.clear();
}
