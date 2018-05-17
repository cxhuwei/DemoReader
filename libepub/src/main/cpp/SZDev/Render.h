#pragma once

#include "SZImage.h"
#include "SZFontManager.h"
#include "ThreadPacket.h"
#include "myfDev/MYFDefine.h"
#include "ReadImageFile.h"
#include "SZDefine.h"

class CRender
{
public:
	CRender(void);
	CRender(const CRender& render);
	~CRender(void);
	int			GetTabSize()const{return m_nTabSize;}
	void		SetTabSize(int val){m_nTabSize = val;}

	int			GetVertSpac()const{return m_nVertSpac;}
	void		SetVertSpac(float val){m_nVertSpac= val;}

	int			GetParagraphSpac()const{return m_nParagraphSpac;}
	void		SetParagraphSpac(float val){m_nParagraphSpac= val;}

	int			GetWordSpac()const{return m_nWordSpac;}
	void		SetWordSpac(int val){m_nWordSpac= val;}

	int			GetMrginLeft()const{return m_nMrginLeft;}
	void		SetMrginLeft(int val);

	int			GetMrginRight()const{return m_nMrginRight;}
	void		SetMrginRight(int val){m_nMrginRight= val;}

	int			GetMrginTop()const{return m_nMrginTop;}
	void		SetMrginTop(int val);

	int			GetMrginBottom()const{return m_nMrginBottom;}	
	void		SetMrginBottom(int val){m_nMrginBottom= val;}

	void		SetClrForeG(myRGBQUAD clr){m_clrForeG=clr;}	
	myRGBQUAD 	GetClrForeG()const{return m_clrForeG;}	

	int			GetOffsetX(){return m_OffsetX;}
	int			GetOffsetY(){return m_OffsetY;}

	void		SetOffsetX(int val){m_OffsetX = val;}
	void		SetOffsetY(int val){m_OffsetY = val;}

	void		AddOffsetY(int val){m_OffsetY += val;}

	void	Init(int iWidth, int iHeight,myDWORD nFormat,myRGBQUAD clrFG);
	int		DrawContentText(SZImageBit24& image,SZFTFont* pChiFont,SZFTFont* pEngFont,int contentId,Content* content,int beginPos,int& endPos,pageWordinfo* pwi,Vt_curPageMultiMedia* cpmm,bool isRender = false);
	int		DrawContentImage(SZImageBit24& pageImage,SZImageBase* image,Content* content,MultimediaPage*  cpmm,bool isRender = false);

	int		DrawImage(SZImageBit24& pageImage,SZImageBase* image,int x,int y);
	int		DrawHighLight(SZImageBit24&  image, SZFTFont* pChiFont,SZFTFont* pEngFont,vector<wordinfo>& vWordInfo,myRGBQUAD bgColor);

private:
	void	Punctuation(std::wstring strFull,int index,std::wstring& strLine,int& deleteCount);//处理标点符号
	int		DrawLine(SZImageBit24& image,SZFTFont* pChiFont,SZFTFont* pEngFont,int& drawX,int& drawY,std::wstring str,int contentId,Content* content,int beginPos,pageWordinfo* pwi,Vt_curPageMultiMedia* cpmm,bool lastCharisReally, bool isNeedJustify = true);
	int		RenderChar(SZImageBit24  &Image,CharCacheNode* pNode,int drawX, int drawY,int defaultAD,myRGBQUAD fontColor, wordinfo& wi);

	int		DrawBackGround( SZImageBit24 &Image,int x,int y,int width,int height,myRGBQUAD bgColor);

	void	InitLineInfo();
protected:
	//页面属性
	int				m_nTabSize; 
	float				m_nVertSpac;
	int				m_nWordSpac; 
	float				m_nParagraphSpac;//段间距
	int				m_nMrginLeft;  
	int				m_nMrginRight; 
	int				m_nMrginTop;  
	int				m_nMrginBottom; 
	int				m_OutDevW; 
	int				m_OutDevH;

	myRGBQUAD				m_clrForeG; //前景色 （字体颜色）如果content没有颜色标记，就使用用户设置字体颜色

	int						m_OffsetX;//绘制横坐标起始位置
	int						m_OffsetY;//绘制纵坐标起始位置	

	int		m_PrevWordCharIndex;//前一个字的face索引值
	int		m_NowWordCharIndex;//当前字的face索引值


	lineinfo m_curlineinfo;//行字信息
	wordinfo m_PreWordinfo;//前一个字的信息

	int		m_curLineMaxHeight;//当前行最大高度

public:
	bool m_iStop;
};
