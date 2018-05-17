// SZFontManager.h: interface for the SZFontManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SZFONTMANAGER_H__A766DCD9_6A94_4C76_8CCF_5460127EB0D9__INCLUDED_)
#define AFX_SZFONTMANAGER_H__A766DCD9_6A94_4C76_8CCF_5460127EB0D9__INCLUDED_

#include "SZFTFont.h"
typedef std::map<int, SZFTFont*> FontList_fontsize;
typedef FontList_fontsize::iterator SZFTFontListIterator;
class SZFontManager  
{
public:
	void ClearFont();
	SZFontManager();
	virtual ~SZFontManager();
	bool Init(string strFontPath);
	SZFTFont* CreateFont(int iFontSize);//创建一个fontSize大小的字体对象
	void End();
private:
	string				m_strFontPath;
	bool				m_bInit;
	FT_Library			m_library;
	FontList_fontsize	m_mFontList;
};
#endif // !defined(AFX_SZFONTMANAGER_H__A766DCD9_6A94_4C76_8CCF_5460127EB0D9__INCLUDED_)
 
