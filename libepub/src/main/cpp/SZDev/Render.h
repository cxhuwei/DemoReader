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
	void	Punctuation(std::wstring strFull,int index,std::wstring& strLine,int& deleteCount);//���������
	int		DrawLine(SZImageBit24& image,SZFTFont* pChiFont,SZFTFont* pEngFont,int& drawX,int& drawY,std::wstring str,int contentId,Content* content,int beginPos,pageWordinfo* pwi,Vt_curPageMultiMedia* cpmm,bool lastCharisReally, bool isNeedJustify = true);
	int		RenderChar(SZImageBit24  &Image,CharCacheNode* pNode,int drawX, int drawY,int defaultAD,myRGBQUAD fontColor, wordinfo& wi);

	int		DrawBackGround( SZImageBit24 &Image,int x,int y,int width,int height,myRGBQUAD bgColor);

	void	InitLineInfo();
protected:
	//ҳ������
	int				m_nTabSize; 
	float				m_nVertSpac;
	int				m_nWordSpac; 
	float				m_nParagraphSpac;//�μ��
	int				m_nMrginLeft;  
	int				m_nMrginRight; 
	int				m_nMrginTop;  
	int				m_nMrginBottom; 
	int				m_OutDevW; 
	int				m_OutDevH;

	myRGBQUAD				m_clrForeG; //ǰ��ɫ ��������ɫ�����contentû����ɫ��ǣ���ʹ���û�����������ɫ

	int						m_OffsetX;//���ƺ�������ʼλ��
	int						m_OffsetY;//������������ʼλ��	

	int		m_PrevWordCharIndex;//ǰһ���ֵ�face����ֵ
	int		m_NowWordCharIndex;//��ǰ�ֵ�face����ֵ


	lineinfo m_curlineinfo;//������Ϣ
	wordinfo m_PreWordinfo;//ǰһ���ֵ���Ϣ

	int		m_curLineMaxHeight;//��ǰ�����߶�

public:
	bool m_iStop;
};
