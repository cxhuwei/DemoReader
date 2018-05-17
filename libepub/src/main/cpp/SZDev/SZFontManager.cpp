// SZFontManager.cpp: implementation of the SZFontManager class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h" 
#include "SZFontManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SZFontManager::SZFontManager()
{
	m_bInit = false;
	m_strFontPath = "";
}

SZFontManager::~SZFontManager()
{
   End();
}

bool SZFontManager::Init(string strFontPath)
{ 
	if(m_strFontPath == strFontPath)
		return true;

	End();

	m_strFontPath = strFontPath;
	if(FT_Init_FreeType(&m_library) != 0)
	{
		m_bInit = false;
	}else
	{
		m_bInit = true;
	}
	return m_bInit;
}

void SZFontManager::End()
{
	if (!m_bInit)
		return;
	
	ClearFont();
	
	if (m_bInit)
	{
		FT_Done_FreeType(m_library);
		m_bInit = false;
	}
}


void SZFontManager::ClearFont()
{
	while (!m_mFontList.empty())
	{
		SZFTFont* pFont = m_mFontList.begin()->second;
		pFont->ClearAllCache();
		m_mFontList.erase(m_mFontList.begin());
		delete pFont;
	}
}

SZFTFont* SZFontManager::CreateFont(int iFontSize )
{
	SZFTFontListIterator pos = m_mFontList.find(iFontSize);
	if (pos == m_mFontList.end())
	{//没有缓存，需要重新创建
		SZFTFont* ptrNewFont = new SZFTFont(m_library);
		if (!ptrNewFont->LoadTTFFontFromDisk(m_strFontPath,iFontSize,iFontSize,false,true))
		{
			delete ptrNewFont;
			ptrNewFont = NULL;
		}
		m_mFontList.insert(pair<int,SZFTFont*>(iFontSize,ptrNewFont));
		return ptrNewFont;
	}
	else
	{
		return pos->second;
	}
	
}
