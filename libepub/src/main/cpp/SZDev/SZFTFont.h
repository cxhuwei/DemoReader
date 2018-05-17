// SZFTFont.h: interface for the SZFTFont class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SZFTFONT_H__C42BABF9_B00E_45EA_9F91_8F3391B884FD__INCLUDED_)
#define AFX_SZFTFONT_H__C42BABF9_B00E_45EA_9F91_8F3391B884FD__INCLUDED_

#include "SZDefine.h"
#include <map>
#include <string>
#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_SIZES_H
#include FT_GLYPH_H
#include FT_FREETYPE_H 
#include FT_STROKER_H 
#include FT_TRUETYPE_IDS_H
typedef wchar_t  myWcharT;
typedef vector<myWcharT> Vt_myString;
// Struct for every cached character
typedef struct tagmyCharCacheNode 
{
	int		    x, y, w, h;						// offset and size of glyph
	myWORD		charCode;						// unicode char value
	myWORD		pixel_mode;	 
	myDWORD		charAdvance;					// advance value
	myDWORD		charGlyphIndex;					// glyph index in the font face
	myDWORD		rowSize;						 
	myDWORD     dataSize;	
	myBYTE     *data;							// bitmap data
	bool		isBold;
	bool		isItalic;
    int         ascender;
    int         descender;
	//add
	FT_GlyphSlot  glyph;
}CharCacheNode;

typedef std::map</*myWORD*/int, CharCacheNode*> CharCacheMap;
typedef CharCacheMap::iterator   CharCacheMapIterator;
class SZDispOut;

class SZFTFont  
{
public:	
	void SetIsItalic(bool bItalic);
	void SetIsBold(bool bBold);
	bool SetFontSize(int iCharWidth,int iCharHeight);
//	CStringList  m_strTemp;
	SZFTFont(FT_Library ftlib);
	virtual ~SZFTFont(); 

	void UnloadFont();	
	void ClearAllCache();
	int  GetFontInfo();
	int  GetHalfWidthAdvance(); 
	int  GetFullWidthAdvance();
    int  GetFullWidthAdvanceCH();
    int  GetHalfWidthAdvanceCH();
	int  GetEngHightAdvance();
	int  GetFontWidth()  const{return m_CharWidth;}
	int  GetFontHeight() const{return m_CharHeight;}
  	int  GetUnderlinePos()  const{return m_iUnderlinePos;}
	float  GetFaceAscender()  const{return m_fFaceAscender;}
	bool IsBold() const{return m_bBold;}
  	bool IsItalic()  const{return m_bItalic;}
	bool IsVertical()  const{return m_bVertical;} 
	bool IsPixelGray() const{return m_bPixelGray;}
	bool IsHasKerning() const{return m_bHasKerning;}
 	bool IsTheFontSupportVERTICAL() const{return m_bsupportVERTICAL;}

 	bool IsCharCached(myWORD charCode);
	bool BuildCharCache(myWORD charCode);
	CharCacheNode* BuildChar(myWORD charCode);
	bool BuildStringCache(const std::wstring& str);
	bool BuildStringCache(const Vt_myString& str);
	bool LoadTTFFontFromDisk(const std::string& strFontPath,int iCharWidth,int iCharHeight, bool bVertical,bool bPixelGray);
	CharCacheNode* GetCharCacheNode(myWORD charCode); 
	CharCacheNode* GetCharCacheNode2(myWORD charCode); 

	int	GetTwoCharKerning(int PrevIndex, int nowIndex);
private:
	static	FT_Matrix		m_matItalic;        // 这个matrix用于设置斜体字	
	static	FT_Library		m_FTLib;	
	FT_Face					m_Face;
	CharCacheMap			m_FontCharCache;	
	
	int						m_CharWidth;		
	int						m_CharHeight;
	int						m_iUnderlinePos;
	float					m_fFaceAscender;
	bool					m_bBold;			// 粗体
	bool					m_bItalic;			// 斜体
	bool					m_bVertical;		// 竖排文字 
	bool					m_bPixelGray;		// true  8bit, fals 1bit
	bool					m_bHasKerning;		// font face has kerning
	bool					m_bsupportVERTICAL; // 该字体是否支持竖版
	friend class SZDispOut; 

	//add by myf
	int  m_HalfWidthAdvance; 
	int  m_FullWidthAdvance;
    int  m_FullWidthAdvanceCH;
    int  m_HalfWidthAdvanceCH;
	int  m_EngHightAdvance;
};
#endif // !defined(AFX_SZFTFONT_H__C42BABF9_B00E_45EA_9F91_8F3391B884FD__INCLUDED_)
