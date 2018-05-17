// SZFTFont.cpp: implementation of the SZFTFont class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h" 
#include "SZFTFont.h"

#include "ThreadPacket.h"
FT_Matrix	SZFTFont::m_matItalic;
FT_Library	SZFTFont::m_FTLib;	
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
THREAD_MUTEX g_lock;
int first2=0;
SZFTFont::SZFTFont(FT_Library ftlib)
{
	m_FTLib		   = ftlib;	
	m_Face		   = 0; 
	
	m_matItalic.xx = 1 << 16;
	m_matItalic.xy = 0x5800;
	m_matItalic.yx = 0;
	m_matItalic.yy = 1 << 16;
  
	m_CharWidth       = 20;
	m_CharHeight      = 20; 
	m_iUnderlinePos	  = 0;
	m_fFaceAscender	  = 1.0f;


	m_bBold		  = false;
	m_bItalic	  = false;
	m_bVertical	  = false;
	m_bHasKerning = false;
	m_bPixelGray  = false;
	m_bsupportVERTICAL=false;
    if (first2 == 0) {
        first2++;
        INITIALIZE_MUTEX(&g_lock); 
    }
    
	
}

SZFTFont::~SZFTFont()
{
	UnloadFont();
}

void SZFTFont::UnloadFont(void)
{
	ClearAllCache();
	if (m_Face)
	{
		FT_Done_Face(m_Face);
		m_Face = 0;
	}
}

void SZFTFont::ClearAllCache(void)
{
	CharCacheNode* pNode=NULL;
// 	while (!m_FontCharCache.empty())
// 	{
// 		pNode = m_FontCharCache.begin()->second;
// 		m_FontCharCache.erase(m_FontCharCache.begin());
//  
// 		if (pNode)
// 		{
// 			if(pNode->data)
// 			{ 
// 				free(pNode->data);
// 				pNode->data=NULL;
// 			}				
// 			delete pNode;
// 			pNode = NULL;
// 		}
// 	}
	CharCacheMap::iterator pos;
	for (pos = m_FontCharCache.begin(); pos != m_FontCharCache.end(); ++pos)
	{
		pNode = pos->second;
		if (pNode)
		{
			if (pNode->data)
			{
				free(pNode->data);
				pNode->data = NULL;
			}
			delete pNode;
			pNode = NULL;
		}
	}
	m_FontCharCache.clear();
}

CharCacheNode* SZFTFont::GetCharCacheNode(myWORD charCode)
{
	int icharCode=0;
	icharCode = (charCode << 1) + (m_bBold?1:0) + (m_bItalic?1:0);
	CharCacheMapIterator it = m_FontCharCache.find(icharCode);
	if(it == m_FontCharCache.end())
		return 0;
	else
	{
		return it->second;	
	}
}
CharCacheNode* SZFTFont::GetCharCacheNode2(myWORD charCode)
{
// 	CharCacheMapIterator it = m_FontCharCache.find(charCode);
// 	if(it == m_FontCharCache.end())
// 		return 0;
// 	else
// 	{
// 		if (it->second->isBold != m_bBold || it->second->isItalic != m_bItalic)
// 		{
// 			return 0;
// 		}
// 		else
// 		{
// 			return it->second;
// 		}
// 		
//	}
	return 0;
}
bool SZFTFont::BuildStringCache(const std::wstring& str)
{
	bool bRet = true;	
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!BuildCharCache(str[i]))
			bRet = false;
	}
	return bRet;
}

bool SZFTFont::BuildStringCache(const Vt_myString& str)
{
	bool bRet = true;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!BuildCharCache(str[i]))
			bRet = false;
	}
	return bRet;
}

bool SZFTFont::IsCharCached(myWORD charCode)
{
	return GetCharCacheNode(charCode) != 0;
}

int SZFTFont::GetHalfWidthAdvance()
{ 
// #if defined (_WIN32) || defined(WIN32)
// 	
// 	CharCacheNode* pNode = BuildChar(L'A');
// #else /* Linux */
// 	myWORD charCode=0x41;//65
// 	
// 	CharCacheNode* pNode = BuildChar(charCode);
// #endif
// 	
// 	int halfwidth;
// 	if(pNode)
// 		halfwidth = pNode->charAdvance;
// 	else 
// 		halfwidth = m_CharWidth / 2;
// 
// 	
// 	if (pNode)
// 	{
// 		if (pNode->data)
// 		{
// 			delete[] pNode->data;
// 			pNode->data = NULL;
// 		}
// 		free(pNode);
// 		pNode = NULL;
// 	}
// 	
// 	return halfwidth;
	return m_HalfWidthAdvance;
}

int SZFTFont::GetFullWidthAdvance()
{  
	return m_FullWidthAdvance;

}
int  SZFTFont::GetFullWidthAdvanceCH()
{
//     
// 	myWORD charCode=0x4e2d;//65
// 	CharCacheNode* pNode = BuildChar(charCode);
// 	int fullwidth;
// 	if(pNode)
// 		fullwidth = pNode->charAdvance;
// 	else 
// 		fullwidth = m_CharWidth;
//     
// 	
// 	if (pNode)
// 	{
// 		if (pNode->data)
// 		{
// 			delete[] pNode->data;
// 			pNode->data = NULL;
// 		}
// 		free(pNode);
// 		pNode = NULL;
// 	}
// 	
// 	return fullwidth;
	return m_FullWidthAdvanceCH;
}
int  SZFTFont::GetHalfWidthAdvanceCH()
{
    //return GetFullWidthAdvanceCH()/2;
	return m_HalfWidthAdvanceCH;
}
int  SZFTFont::GetEngHightAdvance()
{
// #if defined (_WIN32) || defined(WIN32)
// 	
// 	CharCacheNode* pNode = BuildChar(L'j');
// #else /* Linux */
// 	myWORD charCode=0x41;//65
// 	
// 	CharCacheNode* pNode = BuildChar(charCode);
// #endif
// 	
// 	int halfwidth;
// 	if(pNode)
// 		halfwidth = pNode->h;
// 	else 
// 		halfwidth = m_CharWidth;
// 
// 	
// 	if (pNode)
// 	{
// 		if (pNode->data)
// 		{
// 			delete[] pNode->data;
// 			pNode->data = NULL;
// 		}
// 		free(pNode);
// 		pNode = NULL;
// 	}
// 	
// 	return halfwidth;
	return m_EngHightAdvance;
}
bool SZFTFont::BuildCharCache(myWORD charCode)
{
	//charCode是该字符的unicode编码
	if (IsCharCached(charCode))
		return true;
	LOCK_MUTEX(&g_lock);
	CharCacheNode* pNode = new CharCacheNode;
	if (NULL==pNode)
	{
		UNLOCK_MUTEX(&g_lock);
		return false;
	}

	pNode->charCode			= charCode;
	pNode->isBold = m_bBold;
	pNode->isItalic = m_bItalic;
	pNode->charGlyphIndex   = FT_Get_Char_Index(m_Face, charCode);	
	if (pNode->charGlyphIndex == 0)
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return false;
	}
	 

	FT_Int32 flags=(m_bVertical)? FT_LOAD_DEFAULT | FT_LOAD_VERTICAL_LAYOUT:FT_LOAD_DEFAULT;
	if(FT_Load_Char(m_Face, charCode, flags))
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return false;
	}
  
	if(m_bBold){//加粗
		/*pNode->isBold = true;*/
		int strength = 1 << 6;
		FT_Outline_Embolden(&m_Face->glyph->outline, strength);
	}
 
	if(m_bItalic){//斜体	
		/*pNode->isItalic = true;*/
		FT_Outline_Transform(&m_Face->glyph->outline, &m_matItalic);
	}
 
	//准备拷贝glyph image	
	if(FT_Render_Glyph(m_Face->glyph, m_bPixelGray ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO ))
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return false;
	}
 
	int i;
	myBYTE *p, *q;
	FT_GlyphSlot slot =m_Face->glyph;
    pNode->ascender = m_Face->size->metrics.ascender >> 6;
    pNode->descender = m_Face->size->metrics.descender >> 6;
	if (slot->bitmap.width == 0 || slot->bitmap.rows == 0)
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return false;
	}
	pNode->x = slot->bitmap_left;
	pNode->y = slot->bitmap_top;
	pNode->w = slot->bitmap.width;
	pNode->h = slot->bitmap.rows;
	pNode->charAdvance	= (slot->advance.x)/64;	// 获取该文字整个宽度，包含跨距
 
 	pNode->pixel_mode = slot->bitmap.pixel_mode;
	if (pNode->pixel_mode ==FT_PIXEL_MODE_GRAY){
		pNode->rowSize =  pNode->w;
	}else if (pNode->pixel_mode ==FT_PIXEL_MODE_MONO){
		pNode->rowSize = (pNode->w + 7) >> 3;
	}else
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return false;
	}

	pNode->dataSize = (pNode->rowSize) * (pNode->h);
	pNode->data = (myBYTE *)malloc(pNode->dataSize);
	if (NULL==pNode->data)
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return false;
	}
	for (i = 0, p = pNode->data, q=slot->bitmap.buffer; i < pNode->h;++i, p+= (pNode->rowSize), q+=slot->bitmap.pitch)
	{
		memcpy(p, q, (pNode->rowSize));
	}
	int icharCode=0;
	icharCode = (charCode << 1) + (m_bBold?1:0) + (m_bItalic?1:0);
	m_FontCharCache.insert(std::pair<int, CharCacheNode*>(icharCode, pNode)); 
	UNLOCK_MUTEX(&g_lock);
	return true;
}
CharCacheNode* SZFTFont::BuildChar(myWORD charCode)
{
	//charCode是该字符的unicode编码
		//lock
	LOCK_MUTEX(&g_lock);
	CharCacheNode* pNode = new CharCacheNode;
	if (NULL==pNode)
	{
		UNLOCK_MUTEX(&g_lock);
		return NULL;
	}
	
	pNode->charCode			= charCode;
	pNode->isBold = m_bBold;
	pNode->isItalic = m_bItalic;
	pNode->charGlyphIndex   = FT_Get_Char_Index(m_Face, charCode);	
	if (pNode->charGlyphIndex == 0)
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return NULL;
	}
// 	if (previous)
// 	{
// 		FT_Vector delta;
// 		FT_Get_Kerning(m_Face,previous,pNode->charGlyphIndex,ft_kerning_default,&delta);
// 		pNode->delta = delta.x >> 6;
// 	}
	FT_Int32 flags=(m_bVertical)? FT_LOAD_DEFAULT | FT_LOAD_VERTICAL_LAYOUT:FT_LOAD_DEFAULT;
	if(/*FT_Load_Char(m_Face, charCode, flags)*/FT_Load_Glyph(m_Face,pNode->charGlyphIndex, flags))
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return NULL;
	}
	
	if(m_bBold){//加粗
		/*pNode->isBold = true;*/
		int strength = 1 << 6;
		FT_Outline_Embolden(&m_Face->glyph->outline, strength);
	}
	
	if(m_bItalic){//斜体	
		/*pNode->isItalic = true;*/
		FT_Outline_Transform(&m_Face->glyph->outline, &m_matItalic);
	}
	
	//准备拷贝glyph image	
	if(FT_Render_Glyph(m_Face->glyph, m_bPixelGray ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO ))
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return NULL;
	}
	
	int i;
	myBYTE *p, *q;
	FT_GlyphSlot slot =m_Face->glyph;
	if (slot->bitmap.width == 0 || slot->bitmap.rows == 0)
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return NULL;
	}
	pNode->x = slot->bitmap_left;
	pNode->y = slot->bitmap_top;
	pNode->w = slot->bitmap.width;
	pNode->h = slot->bitmap.rows;
	pNode->charAdvance	= (slot->advance.x)/64;	// 获取该文字整个宽度，包含跨距
	
	pNode->pixel_mode = slot->bitmap.pixel_mode;
	if (pNode->pixel_mode ==FT_PIXEL_MODE_GRAY){
		pNode->rowSize =  pNode->w;
	}else if (pNode->pixel_mode ==FT_PIXEL_MODE_MONO){
		pNode->rowSize = (pNode->w + 7) >> 3;
	}else
	{
		UNLOCK_MUTEX(&g_lock);
		delete pNode;
		return NULL;
	}
	
	pNode->glyph = m_Face->glyph;
	pNode->dataSize = (pNode->rowSize) * (pNode->h);
	pNode->data = NULL;

	//previous = pNode->charGlyphIndex;
// 	pNode->data = (myBYTE *)malloc(pNode->dataSize);
// 	if (NULL==pNode->data)
// 	{
// 		UNLOCK_MUTEX(&g_lock);
// 		delete pNode;
// 		return NULL;
// 	}
/*	for (i = 0, p = pNode->data, q=slot->bitmap.buffer; i < pNode->h;++i, p+= (pNode->rowSize), q+=slot->bitmap.pitch)
	{
		memcpy(p, q, (pNode->rowSize));
	}*/
	UNLOCK_MUTEX(&g_lock);
	return pNode;
}
bool SZFTFont::LoadTTFFontFromDisk(const std::string& strFontPath, int iCharWidth,int iCharHeight,bool bVertical,bool bPixelGray)
{	
	UnloadFont();
	m_bsupportVERTICAL=false;
 
	if (FT_New_Face(m_FTLib, strFontPath.c_str(), 0, &m_Face) != 0)
		return false;

// 	if (!m_Face->charmap || !FT_IS_SCALABLE(m_Face))
// 	{ 
// 		return false;
// 	}

	if (!m_Face->charmap )
		return false;
	
// 	if ( !FT_IS_SCALABLE(m_Face))
// 		return false;
 

	if(FT_HAS_VERTICAL(m_Face))
		m_bsupportVERTICAL=true;
	
	if (FT_HAS_KERNING(m_Face))
		m_bHasKerning = true;

	m_CharWidth	 = iCharWidth;
	m_CharHeight = iCharHeight;

	//按pixel大小设置字体尺寸
	if (FT_Set_Pixel_Sizes(m_Face, m_CharWidth, m_CharHeight) != 0)
		return false;




// 	m_bBold			= bBold;
// 	m_bItalic		= bItalic;
	m_bVertical		= bVertical;
	m_bPixelGray	= bPixelGray;
	if (m_bVertical&&!m_bsupportVERTICAL)
	{
		return false;
	}
	
// 	//获取font face信息	
// 	CString sMessage,sTemp;
// 	m_strTemp.RemoveAll();
// 	m_strTemp.AddTail(_T("Face information:")); 
// 	//glyph数量
// 	sMessage.Format(_T("      Totally %d glyphs."), m_Face->num_glyphs);
// 	m_strTemp.AddTail(sMessage); 
// 	//每EM unit数量
// 	sMessage.Format(_T("      %d uints per EM."), m_Face->units_per_EM);
// 	m_strTemp.AddTail(sMessage); 
// 	//char map数量
// 	sMessage.Format(_T("      %d char maps."), m_Face->num_charmaps);
// 	m_strTemp.AddTail(sMessage); 
// 	
// 	//Fix size	这个对于汉字很重要，Fix size对于小字体显示很有帮助
// 	sMessage.Format(_T("      %d fixed sizes:"), m_Face->num_fixed_sizes);
// 	if(m_Face->available_sizes)
// 	{
// 		for(int ii = 0 ; ii <  m_Face->num_fixed_sizes ; ii++)
// 		{
// 			sTemp.Format(_T(" %d"), m_Face->available_sizes[ii].size/64);
// 			sMessage += sTemp;
// 		}
// 	}	
// 	m_strTemp.AddTail(sMessage); 
// 
// 	//下划线位置	 
// 	sMessage.Format(_T("      underline position:%d"), m_iUnderlinePos);
// 	m_strTemp.AddTail(sMessage);
// 	 
// 	//检查是否支持竖排文字
// 	if(m_bsupportVERTICAL)
// 	{
// 		m_strTemp.AddTail(_T("      This font support vertical layout."));	
// 	}else
// 	{
// 		m_strTemp.AddTail(_T("      This font doesn't support vertical layout."));
// 	} 
	GetFontInfo();
	return true;
}


bool SZFTFont::SetFontSize(int iCharWidth, int iCharHeight)
{
	m_CharWidth	 = iCharWidth;
	m_CharHeight = iCharHeight;
	
	//按pixel大小设置字体尺寸
	if (FT_Set_Pixel_Sizes(m_Face, m_CharWidth, m_CharHeight) != 0)
		return false;
	//下划线位置
	m_iUnderlinePos = FT_MulFix(m_Face->underline_position,m_Face->size->metrics.y_scale);
	m_iUnderlinePos =m_iUnderlinePos/64;
	
	//Ascender
    m_fFaceAscender = m_Face->ascender * m_Face->size->metrics.y_scale * float(1.0/64.0) * (1.0f/65536.0f);
	return true;
}

void SZFTFont::SetIsBold(bool bBold)
{
	m_bBold = bBold;
}

void SZFTFont::SetIsItalic(bool bItalic)
{
	m_bItalic = bItalic;
}

int SZFTFont::GetFontInfo()
{

	myWORD charCode=0x0041;//'A'
	CharCacheNode* pNode = BuildChar(charCode);
	if(pNode)
		m_HalfWidthAdvance = pNode->charAdvance;
	else 
		m_HalfWidthAdvance = m_CharWidth / 2;
	m_FullWidthAdvance = m_HalfWidthAdvance * 2;
	m_EngHightAdvance = pNode->h;
	if (pNode)
	{
		if (pNode->data)
		{
			delete[] pNode->data;
			pNode->data = NULL;
		}
		delete pNode;
		pNode = NULL;
	}

	charCode=0x69fd;//'槽'
	pNode = BuildChar(charCode);
	int fullwidth;
	if(pNode)
		m_FullWidthAdvanceCH = pNode->charAdvance;
	else 
		m_FullWidthAdvanceCH = m_CharWidth;
	m_HalfWidthAdvanceCH = m_FullWidthAdvanceCH / 2;
	if (pNode)
	{
		if (pNode->data)
		{
			delete[] pNode->data;
			pNode->data = NULL;
		}
		delete pNode;
		pNode = NULL;
	}
	return 0;
}

int SZFTFont::GetTwoCharKerning( int PrevIndex, int nowIndex )
{
	FT_Bool use_kerning;
	use_kerning = FT_HAS_KERNING( m_Face );//有些字体库不含有字距调整信息
	if (use_kerning && PrevIndex>=0 && nowIndex >= 0)
	{
		FT_Vector delta;
		FT_Get_Kerning( m_Face, PrevIndex, nowIndex,
			FT_KERNING_DEFAULT, &delta );
		return delta.x >> 6;
	}
	else
		return 0;
}
