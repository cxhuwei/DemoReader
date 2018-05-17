// SZRender.h: interface for the SZRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SZDISPOUT_H__D6F7DDD1_E9D9_4166_B50A_01032A556167__INCLUDED_)
#define AFX_SZDISPOUT_H__D6F7DDD1_E9D9_4166_B50A_01032A556167__INCLUDED_
#include "SZImage.h"
#include "SZFontManager.h"
#include "ThreadPacket.h"
#include "myfDev/MYFDefine.h"
#include "ReadImageFile.h"
#include "SZDefine.h"	// Added by ClassView


class SZRenderSimulate  
{
public:	
	SZRenderSimulate();
	virtual ~SZRenderSimulate();	
  
 	int			GetTabSize()const{return m_nTabSize;}
	void		SetTabSize(int val){m_nTabSize = val;}

	int			GetVertSpac()const{return m_nVertSpac;}
	void		SetVertSpac(int val){
									if (val <= 0)
									{
										val = 1;
									}
									m_nVertSpac= val;}

	int			GetWordSpac()const{return m_nWordSpac;}
	void		SetWordSpac(int val){m_nWordSpac= val;}

	int			GetMrginLeft()const{return m_nMrginLeft;}
	void		SetMrginLeft(int val){m_nMrginLeft= val;}

	int			GetMrginRight()const{return m_nMrginRight;}
	void		SetMrginRight(int val){m_nMrginRight= val;}

	int			GetMrginTop()const{return m_nMrginTop;}
	void		SetMrginTop(int val){m_nMrginTop= val;}	

	int			GetMrginBottom()const{return m_nMrginBottom;}	
	void		SetMrginBottom(int val){m_nMrginBottom= val;}

	void		SetClrForeG(myRGBQUAD clr){m_clrForeG=clr;}	
	myRGBQUAD 	GetClrForeG()const{return m_clrForeG;}	

	myDWORD		GetFormat()const{return m_nFormat;}
	void		SetFormat(myDWORD val){m_nFormat = val;}

	bool		Init(int iWidth, int iHeight,myDWORD nFormat,myRGBQUAD clrFG);
	int		    RenderChar(CharCacheNode* pNode, int x, int y,int defaultAD=0); 
	int		    DrawImage(ImgInfo &pimage,int x, int y,int Offset, int &addstuas);
	int		    DrawImage2(ImgInfo &pimage);
	int		    DrawString2(SZFTFont *pFont,SZFTFont *pEnglishFont,const std::wstring& str,int beginPos,int &addstuas,StyleSheet &css,bool iscssEnable);
	int		    DrawString(SZFTFont *pFont,const std::wstring& str,int beginPos,int &addstuas,StyleSheet &css,bool iscssEnable);
	int		    DrawString(SZFTFont *pFont,const Vt_myString& str,int beginPos,int &addstuas,StyleSheet &css,bool iscssEnable);
	int		    DrawString2(SZFTFont *pFont,SZFTFont *pEnglishFont,const Vt_myString& str,int beginPos,int &addstuas,StyleSheet &css,bool iscssEnable);

	int			DrawUnderLine(myPOINT start,myPOINT end,int adjust=0);
	void		DrawBorder(myPOINT LeftTop,myPOINT RightDown);
	bool		IsBlankPage();
	
	int			GetOffsetY();
protected:
	int			checkLineAddWordOK(int  WidthAdvance,int HightAdvance);	 
	int			RetryDrawLine(SZFTFont *pFont,std::wstring& str,unsigned int nowWchar,unsigned int nextWchar,
							int nFullW,int nhalfW,int &x, int &y,int &nowcharW);
	int			RetryDrawLine(SZFTFont *pFont,Vt_myString& str,unsigned int nowWchar,unsigned int nextWchar,
								int nFullW,int nhalfW,int &x, int &y,int &nowcharW);
protected:	
	int				m_nTabSize; 
	int				m_nVertSpac; 
	int				m_nWordSpac; 
	int				m_nMrginLeft;  
	int				m_nMrginRight; 
	int				m_nMrginTop;  
	int				m_nMrginBottom; 
	int				m_nCharContentNum;
	int				m_nImgContentNum;
	myDWORD					m_nFormat;	
	myRGBQUAD				m_clrForeG; 
	int						m_iNowLine;
//	bool					m_bnewline;
	bool					m_blineNew;
	myPOINT					m_nowlineStart;
	myPOINT					m_nowlineEnd;
	int						m_OffsetX; 
	int						m_OffsetY;	
	int						m_OutDevW; 
	int						m_OutDevH;
	float					m_fFaceAscender;
	friend class SZEbookReader; 
};

class SZRender: public SZRenderSimulate
{
public:	
	void CopyOpp(SZRender &opp);
	int SetHighLight(SZImageBit24  &Image, SZFTFont *pFont,SZFTFont *pEFont,vector<wordinfo>& vWordInfo,myRGBQUAD bgColor);
	int DrawBackGround(SZImageBit24  &Image,int x,int y,int width,int height,myRGBQUAD bgColor);
	void InitLineInfo();
	int		    RenderChar(SZImageBit24  &Image,CharCacheNode* pNode, int x, int y,int flag,bool isEngFont = false,int defaultAD=0); 
	int			DrawUnderLine(SZImageBit24  &Image,myPOINT start,myPOINT end,int adjust=0);
	void		DrawBorder(SZImageBit24  &Image,myPOINT LeftTop,myPOINT RightDown);	
	int			DrawImage(SZImageBit24  &Image,SZImageBase*& pimage,int x, int y,int Offset, int imgType, int &addstuas);
	int			DrawLine(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const std::wstring& str,
							int nFullW,int nhalfW,int UnderlinePos,	int x, int y, int zijianju,
							int contentID,int offset,pageWordinfo* pwi,Content* content,bool flag = false);
	int			DrawLine(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const Vt_myString& str,
						int nFullW,int nhalfW,int UnderlinePos,	int x, int y, int zijianju,
						int contentID,int offset,pageWordinfo* pwi,Content* content,bool flag = false);

	int		    DrawString(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const std::wstring& str,int beginPos,
				int &addstuas,StyleSheet &css,bool iscssEnable,int contentID,pageWordinfo* pwi);
	int		    DrawString2(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const std::wstring& str,int beginPos,
				int &addstuas,StyleSheet &css,bool iscssEnable,int contentID,pageWordinfo* pwi,Vt_curPageInterLinks* Cpil,string url,Content* content);
	int		    DrawString(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const Vt_myString& str,int beginPos,
				int &addstuas,StyleSheet &css,bool iscssEnable,int contentID,pageWordinfo* pwi);
	int		    DrawString2(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const Vt_myString& str,int beginPos,
				int &addstuas,StyleSheet &css,bool iscssEnable,int contentID,pageWordinfo* pwi,Vt_curPageInterLinks* Cpil,string url,Content* content);
public:
	lineinfo m_curlineinfo;
};
#endif // !defined(AFX_SZDISPOUT_H__D6F7DDD1_E9D9_4166_B50A_01032A556167__INCLUDED_)
