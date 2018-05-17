// SZRender.cpp: implementation of the SZRender class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h" 
#include "SZRender.h"
#include "MAlgorithm.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#include "ThreadPacket.h"
extern bool g_IsDrawing;
#ifndef _WIN32
extern THREAD_MUTEX g_ClacpageMutex;
extern pthread_cond_t g_cond;
#endif

SZRenderSimulate::SZRenderSimulate()
{
	m_nTabSize		=4; 
	m_nWordSpac		=0;
	m_nVertSpac		=1; 
	m_nMrginLeft	=24; 
	m_nMrginRight	=24;
	m_nMrginTop		=12;  
	m_nMrginBottom	=12;
	m_iNowLine		=0;
	m_nCharContentNum	=0;
	m_nImgContentNum	=0;

// 	m_nMrginLeft	=8; 
// 	m_nMrginRight	=0;
// 	m_nMrginTop		=0;  
// 	m_nMrginBottom	=0;


	m_OffsetX	   = 0; 
	m_OffsetY	   = 0;

	m_OutDevW	   = 0;
	m_OutDevH	   = 0; 

	m_nFormat	   = 0; 
	m_blineNew	   = true;
//	m_bnewline	   = false;
	m_clrForeG	   = SZImageBase::MakeRGBQuad(0,0,0);
	memset(&m_nowlineStart,0,sizeof(m_nowlineStart));
	memset(&m_nowlineEnd,0,sizeof(m_nowlineEnd));

}

SZRenderSimulate::~SZRenderSimulate()
{

}

bool	SZRenderSimulate::IsBlankPage()
{
// 	if (m_nCharContentNum==0&&m_nImgContentNum==0)
// 	{
// 		return true;
// 	}
	return false;
}

bool SZRenderSimulate::Init(int iWidth, int iHeight,myDWORD nFormat,myRGBQUAD clrFG)
{
	m_OutDevW	   = iWidth;
	m_OutDevH	   = iHeight;
	m_nFormat	   = nFormat;
	m_clrForeG	   = clrFG;
	m_iNowLine	   =0;
	m_nCharContentNum	=0;
	m_nImgContentNum	=0;
	m_blineNew	   = true;
//	m_bnewline	   = false;
	m_OffsetX	   = m_nMrginLeft; 
	m_OffsetY	   = m_nMrginTop; 
	memset(&m_nowlineStart,0,sizeof(m_nowlineStart));
	memset(&m_nowlineEnd,0,sizeof(m_nowlineEnd));
	return true;
}

int SZRenderSimulate::checkLineAddWordOK(int WidthAdvance,int HightAdvance)
{
	int temp;
	if (WidthAdvance>0)
	{
		temp=m_OffsetX +WidthAdvance;
		if (temp>=(m_OutDevW-m_nMrginRight))
		{
			return -1;
		}
	}

	if (HightAdvance>0)
	{
		temp =  m_OffsetY +HightAdvance;
		if (temp>=(m_OutDevH-m_nMrginBottom-HightAdvance))
		{
			return -2;
		}
	}
	return 1;
}

int SZRender::DrawUnderLine(SZImageBit24  &Image,myPOINT start,myPOINT end,int adjust)
{ 
	if (adjust==0)
	{
		return Image.DrawLine(start,end,m_clrForeG);
	}else
	{
		start.y-=adjust; 
		end.y-=adjust; 
		return Image.DrawLine(start,end,m_clrForeG);
	}	  
}

int SZRenderSimulate::DrawUnderLine(myPOINT start,myPOINT end,int adjust)
{ 
	if (adjust==0)
	{
		return 1;
	}else
	{
		start.y-=adjust; 
		end.y-=adjust; 
		return 1;
	}	  
}

void SZRender::DrawBorder(SZImageBit24  &Image,myPOINT LeftTop,myPOINT RightDown)
{
	myPOINT temp=RightDown;
	temp.x=LeftTop.x;
	/***************************************/
	/*                                      */
	/*            .**************           */
	/*            .             *           */
	/*            .             *           */
	/*            .             *           */
	/*            .**************           */
	/*                                      */
	/***************************************/
	DrawUnderLine(Image,LeftTop,temp);
	/***************************************/
	/*                                      */
	/*            .**************           */
	/*            .             *           */
	/*            .             *           */
	/*            .             *           */
	/*            ...............           */
	/*                                      */
	/***************************************/
	DrawUnderLine(Image,temp,RightDown);
	temp.x=RightDown.x;
	temp.y=LeftTop.y;
	/***************************************/
	/*                                      */
	/*            ...............           */
	/*            .             *           */
	/*            .             *           */
	/*            .             *           */
	/*            ...............           */
	/*                                      */
	/***************************************/
	DrawUnderLine(Image,LeftTop,temp);
	/***************************************/
	/*                                      */
	/*            ...............           */
	/*            .             .           */
	/*            .             .           */
	/*            .             .           */
	/*            ...............           */
	/*                                      */
	/***************************************/
	DrawUnderLine(Image,temp,RightDown);
}

void SZRenderSimulate::DrawBorder(myPOINT LeftTop,myPOINT RightDown)
{
	myPOINT temp=RightDown;
	temp.x=LeftTop.x;
	DrawUnderLine(LeftTop,temp);
	DrawUnderLine(temp,RightDown);
	temp.x=RightDown.x;
	temp.y=LeftTop.y;
	DrawUnderLine(LeftTop,temp);
	DrawUnderLine(temp,RightDown);
}

int  SZRender::RenderChar(SZImageBit24  &Image,CharCacheNode* pNode, int Offsetx, int Offsety,int flag,bool isEngFont,int defaultAD)
{
	int x,y;
	myBYTE pixV=0;
	if (!pNode /*|| !pNode->data*/)
	{ 
		return -1;
	}
	if (defaultAD==0)
	{
		defaultAD=pNode->charAdvance;
	}
	unsigned char *pSrc = isEngFont?pNode->data:pNode->glyph->bitmap.buffer;
	if (pSrc == NULL)
	{
		return -1;
	}
	int x_begin = 0;
	int x_end = pNode->w;
//  	if (flag == 1||flag==2 )
// 	{
//  		x_begin = pNode->x; 
// 		x_end = pNode->w/2;
//	}
// 	if (flag == 2)
// 	{
// 		x_begin = pNode->w/2;
// 		x_end = pNode->w;
// 	}
	if (pNode->pixel_mode==FT_PIXEL_MODE_MONO)
	{
		for( x = x_begin ; x <  x_end; x++)
		{
			for(y =0 ; y < pNode->h; y++)
			{		
				
				pixV = isEngFont?pSrc[y * pNode->rowSize + x/ 8]:pSrc[x * pNode->glyph->bitmap.pitch + y/8];
			
				Image.SetDesPixel(Offsetx+x+pNode->x,
					m_OutDevH-1-(Offsety+defaultAD-(pNode->y-pNode->h)+y),
					m_clrForeG,
					((pixV & (0x80 >> (x & 7))))?255:0);
			}
		}

	}
	else if (pNode->pixel_mode==FT_PIXEL_MODE_GRAY)
	{

		for( x = x_begin ; x <  x_end; x++)
		{
			for( y =0 ; y < pNode->h; y++)
			{				
				 pixV=pSrc[y * pNode->rowSize + x];
				 Image.SetDesPixel(Offsetx+x+pNode->x,					
				 m_OutDevH-1-(Offsety+(defaultAD-pNode->y)+y),m_clrForeG,pixV);			
				
			}

		}

	}
	else 
	{
		return -2;
	} 
	return 0;
}

int  SZRenderSimulate::RenderChar(CharCacheNode* pNode, int Offsetx, int Offsety,int defaultAD)
{
	if (!pNode || !pNode->data)
	{ 
		return -1;
	}
	return 0;
}

//返回0 不在该行添加nowWchar字。
//返回1  在改换添加
int  SZRenderSimulate::RetryDrawLine(SZFTFont *pFont,std::wstring& str,unsigned int nowWchar,unsigned int nextWchar,
									 int nFullW,int nhalfW,int &x, int &y,int &nowcharW)
{
// 	中华人民共和国国家标准标点符号用法	（中华人民共和国国家标准 GB/T 15834-1995）		
// 	国家技术监督局1995年12月13日发布，1996年6月1日施行
// 	5.1　句号、问号、叹号、逗号、顿号、分号和冒号一般占一个字的位置，居左偏下，不出现在一行之首。 
// 	5.2　引号、括号、书名号的前一半不出现在一行之末，后一半不出现在一行之首。 
// 	「」『』
// 0x300c  0x300d  0x300e  0x300f
	//不出现在一行之首。
	//  」  0x300d  』  0x300f
	// 	,	0x002c  ，	0xff0c // 	,	2c00    ，	0cff
	// 	.	0x002e 	。	0x3002 // 	.	2e00 	。	0230
	// 	!	0x0021	！	0xff01 // 	!	2100	！	01ff
	// 	?	0x003f  ？	0xff1f // 	?	3f00    ？	1fff
	// 				、	0x3001 // 				、	0130
	// 	:	0x003a  ：	0xff1a // 	:	3a00   	：	1aff
	// 	;	0x003b  ；	0xff1b // 	;	3b00    ；	1bff  
	// 	'	0x0027	’	0x2019 // 	'	2700	’	1920
	// 	"	0x0022	”	0x201d // 	"	2200	”	1d20
	// 	)	0x0029	）	0xff09 // 	)	2900	）	09ff
	// 	]	0x005d	】	0x3011 // 	]	5d00	】	1130
	// 	}	0x007d	｝	0xff5d // 	}	7d00	｝	5dff
	// 	>	0x003e	》	0x300b // 	>	3e00	》	0b30  	
	//不出现在一行之末。
	// 「  0x300c  『  0x300e
	//				‘	0x2018 //				‘	1820
	//				“	0x201c //				“	1c20
	// 	(	0x0028	（	0xff08 // 	(	2800	（	08ff
	// 	[	0x005b	【	0x3010 // 	[	5b00	【	1030
	// 	{	0x007b	｛	0xff5b // 	{	7b00	｛	5bff
	// 	<	0x003c	《	0x300a // 	<	3c00	《	0a30
// 	L'中' 0x4e2d 20013
// 		L'A'   0x41   65
// 		L' ' 	0x20 32
// 		L'　'	0x3000	12288	 
// 		L'\t'   0x9 9
// 		L'\r' 	0xd 13		 		
// L'\n'	0xa 10



	int tempx,tempy;
	int OffsetX=x;
	int OffsetY=y;
	nowcharW=0;
	//CharCacheNode* pNode =NULL;

	if (!pFont)
		return -1;
// 	m_blineNew	   = true;
   
	if (nowWchar==0x20 )//L' '
	{
		nowcharW = nhalfW;
		OffsetX += nhalfW;	
	}else if (nowWchar==0x3000)//L'　'
	{
		nowcharW = nFullW;
		OffsetX += nFullW;
	}else if (nowWchar==0x9)// L'\t'
	{
		nowcharW = nFullW * m_nTabSize;
		OffsetX += nFullW * m_nTabSize;
	}else
	{
		
		if (((nowWchar >> 8) & 0xff) == 0)//半角
		{
			nowcharW = nhalfW;
			OffsetX += nhalfW;
			//return -2;			
		}
		else 
		{
			nowcharW = nFullW;
			OffsetX += nFullW;
		}
	}
	 
	if (OffsetX>(m_OutDevW-m_nMrginRight))
	{//超过右边界了		
		if (str.length()==0)
		{//要考虑这个是该行首个字的情况
			x=OffsetX;
			y=OffsetY;
			str+=(wchar_t)nowWchar;
			return 1;
		}
  
		if((nowWchar==0x002c)||(nowWchar==0xff0c)||(nowWchar==0x002e)||(nowWchar==0x3002)||(nowWchar==0x0021)
			||(nowWchar==0xff01)||(nowWchar==0x003f)||(nowWchar==0xff1f)||(nowWchar==0x3001)||(nowWchar==0x003a)
			||(nowWchar==0xff1a)||(nowWchar==0x003b)||(nowWchar==0xff1b)||(nowWchar==0x0027)||(nowWchar==0x2019)
			||(nowWchar==0x0022)||(nowWchar==0x201d)||(nowWchar==0x0029)||(nowWchar==0xff09)||(nowWchar==0x005d)
			||(nowWchar==0x3011)||(nowWchar==0x007d)||(nowWchar==0xff5d)||(nowWchar==0x003e)||(nowWchar==0x300b)
			||(nowWchar==0x300d)||(nowWchar==0x300f))
		{		

			x=OffsetX;
			y=OffsetY;
			str+=(wchar_t)nowWchar;
			return 1;
		}
		else return 0;
	}

	if (nextWchar==0)
	{
		x=OffsetX;
		y=OffsetY;
		str+=(wchar_t)nowWchar;
		return 1;
	}

	tempx=OffsetX;
	tempy=OffsetY;
	if (nextWchar==0x20) //L' '
	{
		OffsetX += nhalfW;	
	}else if (nextWchar==0x3000)//L'　'
	{
		OffsetX += nFullW;
	}else if (nextWchar==0x9 )//L'\t'
	{
		OffsetX += nFullW * m_nTabSize;
	}else
	{
		
		if (((nowWchar >> 8) & 0xff) == 0)
		{
			OffsetX += nhalfW;
			//return -2;			
		}
		else OffsetX += nFullW;
	}
	
	if (OffsetX>(m_OutDevW-m_nMrginRight))
	{		
		if((nextWchar==0x002c)||(nextWchar==0xff0c)||(nextWchar==0x002e)||(nextWchar==0x3002)||(nextWchar==0x0021)
			||(nextWchar==0xff01)||(nextWchar==0x003f)||(nextWchar==0xff1f)||(nextWchar==0x3001)||(nextWchar==0x003a)
			||(nextWchar==0xff1a)||(nextWchar==0x003b)||(nextWchar==0xff1b)||(nextWchar==0x0027)||(nextWchar==0x2019)
			||(nextWchar==0x0022)||(nextWchar==0x201d)||(nextWchar==0x0029)||(nextWchar==0xff09)||(nextWchar==0x005d)
			||(nextWchar==0x3011)||(nextWchar==0x007d)||(nextWchar==0xff5d)||(nextWchar==0x003e)||(nextWchar==0x300b)
			||(nowWchar==0x300d)||(nowWchar==0x300f))
		{		
			if (OffsetX>=m_OutDevW)
				return 0;		
		}	
		x=tempx;
		y=tempy;
		str+=(wchar_t)nowWchar;
		return 1;
	}

	if ((OffsetX+nFullW)>(m_OutDevW-m_nMrginRight))
	{
		if ((nextWchar==0x2018)||(nextWchar==0x201c)||(nextWchar==0x0028)||(nextWchar==0xff08)||(nextWchar==0x005b)
			||(nextWchar==0x3010)||(nextWchar==0x007b)||(nextWchar==0xff5b)||(nextWchar==0x003c)||(nextWchar==0x300a)
			||(nextWchar==0x300c)||(nextWchar==0x300e))
		{
			if(!((nowWchar==0x002c)||(nowWchar==0xff0c)||(nowWchar==0x002e)||(nowWchar==0x3002)||(nowWchar==0x0021)
				||(nowWchar==0xff01)||(nowWchar==0x003f)||(nowWchar==0xff1f)||(nowWchar==0x3001)||(nowWchar==0x003a)
				||(nowWchar==0xff1a)||(nowWchar==0x003b)||(nowWchar==0xff1b)||(nowWchar==0x0027)||(nowWchar==0x2019)
				||(nowWchar==0x0022)||(nowWchar==0x201d)||(nowWchar==0x0029)||(nowWchar==0xff09)||(nowWchar==0x005d)
				||(nowWchar==0x3011)||(nowWchar==0x007d)||(nowWchar==0xff5d)||(nowWchar==0x003e)||(nowWchar==0x300b))
				||(nowWchar==0x300d)||(nowWchar==0x300f))
			{
				return 0;	
			} 
		}
	}
 
	x=tempx;
	y=tempy;
	str+=(wchar_t)nowWchar;
	return 1; 
}

int	SZRenderSimulate::RetryDrawLine(SZFTFont *pFont,Vt_myString& str,unsigned int nowWchar,unsigned int nextWchar,
							int nFullW,int nhalfW,int &x, int &y,int &nowcharW)
{
	int tempx,tempy;
	int OffsetX=x;
	int OffsetY=y;
	nowcharW=0;
//	CharCacheNode* pNode =NULL;

	if (!pFont)
		return -1;
// 	m_blineNew	   = true;

	if (nowWchar==0x20 )//L' '
	{
		nowcharW = nhalfW;
		OffsetX += nhalfW;
	}else if (nowWchar==0x3000)//L'　'
	{
		nowcharW = nFullW;
		OffsetX += nFullW;
	}else if (nowWchar==0x9)// L'\t'
	{
		nowcharW = nFullW * m_nTabSize;
		OffsetX += nFullW * m_nTabSize;
	}else
	{
		
		if (((nowWchar >> 8) & 0xff) == 0)
		{
			nowcharW = nhalfW;
			OffsetX += nhalfW;
			//return -2;
		}
		else
		{
			nowcharW = nFullW;
			OffsetX += nFullW;
		}
	}

	if (OffsetX>(m_OutDevW-m_nMrginRight))
	{
		if (str.size()==0)
		{//要考虑这个是该行首个字的情况
			x=OffsetX;
			y=OffsetY;
			str.push_back((wchar_t)nowWchar);
			return 1;
		}

		if((nowWchar==0x002c)||(nowWchar==0xff0c)||(nowWchar==0x002e)||(nowWchar==0x3002)||(nowWchar==0x0021)
			||(nowWchar==0xff01)||(nowWchar==0x003f)||(nowWchar==0xff1f)||(nowWchar==0x3001)||(nowWchar==0x003a)
			||(nowWchar==0xff1a)||(nowWchar==0x003b)||(nowWchar==0xff1b)||(nowWchar==0x0027)||(nowWchar==0x2019)
			||(nowWchar==0x0022)||(nowWchar==0x201d)||(nowWchar==0x0029)||(nowWchar==0xff09)||(nowWchar==0x005d)
			||(nowWchar==0x3011)||(nowWchar==0x007d)||(nowWchar==0xff5d)||(nowWchar==0x003e)||(nowWchar==0x300b)
			||(nowWchar==0x300d)||(nowWchar==0x300f))
		{
// 			if (OffsetX<m_OutDevW)
// 			{
// 				return 1;
// 			}
			x=OffsetX;
			y=OffsetY;
			str.push_back((wchar_t)nowWchar);
			return 1;
		}
		else return 0;
	}

	if (nextWchar==0)
	{
		x=OffsetX;
		y=OffsetY;
		str.push_back((wchar_t)nowWchar);
		return 1;
	}

	tempx=OffsetX;
	tempy=OffsetY;
	if (nextWchar==0x20) //L' '
	{
		OffsetX += nhalfW;
	}else if (nextWchar==0x3000)//L'　'
	{
		OffsetX += nFullW;
	}else if (nextWchar==0x9 )//L'\t'
	{
		OffsetX += nFullW * m_nTabSize;
	}else
	{
		//pNode = pFont->GetCharCacheNode(nextWchar);
		if (((nowWchar >> 8) & 0xff) == 0)
		{
			OffsetX += nhalfW;
			//return -2;
		}
		else OffsetX += nFullW;
	}

	if (OffsetX>(m_OutDevW-m_nMrginRight))
	{
		if((nextWchar==0x002c)||(nextWchar==0xff0c)||(nextWchar==0x002e)||(nextWchar==0x3002)||(nextWchar==0x0021)
			||(nextWchar==0xff01)||(nextWchar==0x003f)||(nextWchar==0xff1f)||(nextWchar==0x3001)||(nextWchar==0x003a)
			||(nextWchar==0xff1a)||(nextWchar==0x003b)||(nextWchar==0xff1b)||(nextWchar==0x0027)||(nextWchar==0x2019)
			||(nextWchar==0x0022)||(nextWchar==0x201d)||(nextWchar==0x0029)||(nextWchar==0xff09)||(nextWchar==0x005d)
			||(nextWchar==0x3011)||(nextWchar==0x007d)||(nextWchar==0xff5d)||(nextWchar==0x003e)||(nextWchar==0x300b)
			||(nowWchar==0x300d)||(nowWchar==0x300f))
		{
			if (OffsetX>=m_OutDevW)
				return 0;
		}
		x=tempx;
		y=tempy;
		str.push_back((wchar_t)nowWchar);
		return 1;
	}

	if ((OffsetX+nFullW)>(m_OutDevW-m_nMrginRight))
	{
		if ((nextWchar==0x2018)||(nextWchar==0x201c)||(nextWchar==0x0028)||(nextWchar==0xff08)||(nextWchar==0x005b)
			||(nextWchar==0x3010)||(nextWchar==0x007b)||(nextWchar==0xff5b)||(nextWchar==0x003c)||(nextWchar==0x300a)
			||(nextWchar==0x300c)||(nextWchar==0x300e))
		{
			if(!((nowWchar==0x002c)||(nowWchar==0xff0c)||(nowWchar==0x002e)||(nowWchar==0x3002)||(nowWchar==0x0021)
				||(nowWchar==0xff01)||(nowWchar==0x003f)||(nowWchar==0xff1f)||(nowWchar==0x3001)||(nowWchar==0x003a)
				||(nowWchar==0xff1a)||(nowWchar==0x003b)||(nowWchar==0xff1b)||(nowWchar==0x0027)||(nowWchar==0x2019)
				||(nowWchar==0x0022)||(nowWchar==0x201d)||(nowWchar==0x0029)||(nowWchar==0xff09)||(nowWchar==0x005d)
				||(nowWchar==0x3011)||(nowWchar==0x007d)||(nowWchar==0xff5d)||(nowWchar==0x003e)||(nowWchar==0x300b))
				||(nowWchar==0x300d)||(nowWchar==0x300f))
			{
				return 0;
			}
		}
	}

	x=tempx;
	y=tempy;
	str.push_back((wchar_t)nowWchar);
	return 1;
}

int  SZRender::DrawLine(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const std::wstring& str,
						int nFullW,int nhalfW,int UnderlinePos,	int x, int y, int zijianju,int contentID,int offset,pageWordinfo* pwi,Content* content,bool flag)
{ 	
	if (!pFont)
		return -1;
	if (str.size() == 0)
	{
		return 0;
	}
	int nEFullW=pEnglishFont->GetFullWidthAdvance();
	int tempFullW = nFullW;
	int nEhalfW=pEnglishFont->GetHalfWidthAdvance();
	int nEhalfH=pEnglishFont->GetEngHightAdvance();
	bool bnewline=false;
	int countWritechar=0;
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	m_OffsetX=x;
	m_OffsetY=y;

	bool isFirstSpace;
	int topSpaceNumber(0);
	if (offset == 0)
	{
		isFirstSpace = true;
	}  
	else
		isFirstSpace = false;

	if (y != m_curlineinfo.y)
	{
		if (m_curlineinfo.words.size() > 0)
		{
			pwi->lines.push_back(m_curlineinfo);
		}	
		InitLineInfo();
		m_curlineinfo.x = x;
		m_curlineinfo.y = y;
	}

	CharCacheNode* pNode =NULL;
	int countLength = m_OffsetX;
	int wordCount(0);
	bool lastwordIsEng(false);
	size_t i;
//	long t1 = GetTickCount();
	for (i = 0; i < str.length(); i++)
	{
		myWORD nowWchar=0; 
	    nowWchar=str[i];
		switch (nowWchar)
		{
		case 0x20:		 //L' '
			countLength += nhalfW;
			countLength += zijianju;
			wordCount++;
			break;			
		case 0x3000	:	//L'　'
			countLength += nFullW;
			countLength += zijianju;
			wordCount++;
			break;			
		case 0x9 :		//L'\t'
			countLength += nFullW * m_nTabSize;
			break;		
		case 0xa:		//L'\n'
			break;
		default:
			if (isEngChar(nowWchar))
			{
				lastwordIsEng=true;
			
				pEnglishFont->BuildCharCache(nowWchar);
				CharCacheNode* temp = pEnglishFont->GetCharCacheNode(nowWchar);
				countLength += temp->charAdvance;
				countLength -= zijianju;
			}
			else
			{
				lastwordIsEng=false;
				if (((nowWchar >> 8) & 0xff) == 0)//半角
				{
					countLength += nhalfW;
				}
				else 
				{
					int re = returnCharLeftRight(nowWchar);
					if (re == 1 || re == 2)
					{
						pNode = pFont->BuildChar(nowWchar);
						if (pNode == NULL)
						{
							countLength += nhalfW;
						}
						else
							countLength +=  pNode->x+pNode->w;
						if (pNode)
						{
							delete pNode;
							pNode = NULL;
						}
					}
					else
						countLength += nFullW;
				}
				wordCount++;
			}
			countLength += zijianju;
			break;
		}
		
	}
	if(lastwordIsEng)
		wordCount++;
	int shengyu = m_OutDevW - m_nMrginRight - countLength;
	int tianchong(0);
	int tianchongzhi(1);
	if (shengyu < 0 )
	{
		tianchongzhi = -1;
		/*shengyu *= -1;*/
	}
	if (flag && (wordCount - 1) > 0)
	{
		int a = shengyu / (wordCount - 1);
		zijianju += a;
		tianchong = abs(shengyu) % (wordCount - 1);
	}
//	long t2 = GetTickCount();
//	TRACE("drawlien precalc use time:%ld\n",t2-t1);
	bool isEngFont;
	for (i = 0; i < str.length(); i++)
	{
		myWORD nowWchar=0; 
	    nowWchar=str[i];

		wordinfo word;
		word.titleType = content->titleType;
		word.textStyle = content->textStyle;
		word.lineIndex = pwi->lines.size();
		word.WordUnicod = nowWchar;
		switch (nowWchar)
		{
		case 0x20:		 //L' '
			if (isFirstSpace)
			{
				topSpaceNumber++;
			}
			else
			{
				word.width = nhalfW;
				word.height = nEhalfH;//pFont->GetFontHeight();
				word.x = m_OffsetX;
				word.y = m_OffsetY;
			}
			m_OffsetX += nhalfW;
			if (tianchong-- > 0)
			{
				m_OffsetX += tianchongzhi;
			}
			m_OffsetX += zijianju;
			break;			
		case 0x3000	:	//L'　'
			if (isFirstSpace)
			{
				topSpaceNumber++;
			}
			else
			{
				word.width = tempFullW;
				word.height = nEhalfH;//pFont->GetFontHeight();
				word.x = m_OffsetX;
				word.y = m_OffsetY;
			}
			m_OffsetX += tempFullW;
			if (tianchong-- > 0)
			{
				m_OffsetX += tianchongzhi;
			}
			m_OffsetX += zijianju;
			break;			
		case 0x9 :		//L'\t'
			word.x = m_OffsetX;
			word.y = m_OffsetY;
			m_OffsetX += tempFullW * m_nTabSize;
			word.width = tempFullW * m_nTabSize;
			word.height = pFont->GetFontHeight();
			if (tianchong-- > 0)
			{
				m_OffsetX += tianchongzhi;
			}
			break;		
		case 0xa:		//L'\n'
			m_nowlineEnd.x=m_OffsetX;
			m_nowlineEnd.y=m_OffsetY+tempFullW;
			goto exithah;
			break;
		default:
			isFirstSpace = false;
			if (!isEngChar(nowWchar))
			{
                //pFont->BuildCharCache(nowWchar);
				pNode = /*pFont->GetCharCacheNode(nowWchar);*/pFont->BuildChar(nowWchar);
				nFullW = tempFullW;
				if (tianchong-- > 0)
				{
					m_OffsetX += tianchongzhi;
				}
				isEngFont = false;
			}
			else
			{
				pEnglishFont->BuildCharCache(nowWchar);
				pNode = pEnglishFont->GetCharCacheNode(nowWchar);
				nFullW = nEFullW;
				isEngFont = true;
			}

			if (!pNode /*|| !pNode->data*/)
			{	 
				m_OffsetX += nFullW;
				break;
				continue;			
			}else		
			{
				if (!bnewline)
				{
					bnewline=true;
					m_nowlineStart.x=m_OffsetX;
					m_nowlineStart.y=m_OffsetY+nFullW;
				}		

				if (((nowWchar >> 8) & 0xff) != 0 && i == str.size() - 1)
				{
					int devright = m_OutDevW - m_nMrginRight;
					int ww;
					int re = returnCharLeftRight(nowWchar);
					if ( re == 1 || re == 2)
					{
						ww =  (pNode->x+pNode->w);
						if (nowWchar == 0xff0c || nowWchar == 0x3002)
						{
							m_OffsetX -= pNode->x;
						}
					}
					else
					{
						ww = nFullW;
					}
					int avage = devright - (m_OffsetX +  ww);
					if (avage < (ww / 2))
					{
						m_OffsetX += avage;  
					}
				}
				else
				{
					if (returnCharLeftRight(nowWchar) == 1)
					{
						m_OffsetX -= pNode->x;
					}
// 					else if (returnCharLeftRight(nowWchar) == 2)
// 					{
// 						m_OffsetX += pNode->charAdvance - pNode->w;
// 					}
				}
				RenderChar(Image,pNode, m_OffsetX, m_OffsetY,returnCharLeftRight(nowWchar),isEngFont,tempFullW);
				word.x = m_OffsetX;
				word.y = m_OffsetY + tempFullW-pNode->y -1;
				if (isEngChar(nowWchar))
				{
					m_OffsetX += pNode->charAdvance;
					
					word.height = pNode->y + pNode->h;
					word.width = pNode->charAdvance;
				}
				else
				{
					if (((nowWchar >> 8) & 0xff) == 0)//半角
					{

						m_OffsetX += nhalfW;
						word.width = nhalfW;
					}
					else 
					{
						int re = returnCharLeftRight(nowWchar);
						if ( re == 1 || re == 2)
						{
							m_OffsetX += (pNode->x+pNode->w);
							word.width = (pNode->x+pNode->w);
							if (re == 1)
							{
								m_OffsetX += pNode->x;
							}
// 							else if (re == 2)
// 							{
// 								m_OffsetX -= (pNode->charAdvance - pNode->w);
// 							}
						}
						else
						{
							m_OffsetX += nFullW;
							word.width = nFullW;
						}
					}
					word.height = pNode->h;///nFullW;
					/*free(pNode->data);
					pNode->data=NULL;*/
					delete pNode;
					pNode = NULL;
					m_OffsetX += zijianju;
				}
				
			}
		} 

		if (!isFirstSpace)//每个content开头的空格不需要记录信息
		{
			word.contentID = contentID;
			m_curlineinfo.Width += word.width;
			word.offset = i + offset/* - topSpaceNumber*/;
			//word.y = m_OffsetY;
			
			if (m_curlineinfo.Height < word.height)
			{
				m_curlineinfo.Height = word.height;
			}
			m_curlineinfo.words.push_back(word);
		}
		
		countWritechar++;
	} 
//	t1 = GetTickCount();
//	TRACE("drawline realdraw use time:%ld\n",t1-t2);
	m_curlineinfo.RealwordNum += str.length();
exithah:
	if (countWritechar>0)
	{		
		m_nowlineEnd.x=m_OffsetX;
		m_nowlineEnd.y=m_OffsetY+nFullW;
		if((m_nFormat & DT_EX_UNDERLINE) && bnewline) 
		{
			if(m_nowlineEnd.y != m_nowlineStart.y)
			{
				m_nowlineEnd.y = m_nowlineStart.y;
			}
			DrawUnderLine(Image,m_nowlineStart,m_nowlineEnd,UnderlinePos);
		}	
	}
	m_OffsetX=OffsetX;
	m_OffsetY=OffsetY;
	return 0;
}



int  SZRender::DrawLine(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const Vt_myString& str,
						int nFullW,int nhalfW,int UnderlinePos,	int x, int y, int zijianju,int contentID,int offset,pageWordinfo* pwi,Content* content,bool flag)
{ 	

	if (!pFont)
		return -1;
	if (str.size() == 0)
	{
		return 0;
	}
	int nEFullW=pEnglishFont->GetFullWidthAdvance();
	int tempFullW = nFullW;
	int nEhalfW=pEnglishFont->GetHalfWidthAdvance();
	int nEhalfH=pEnglishFont->GetEngHightAdvance();
	bool bnewline=false;
	int countWritechar=0;
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	m_OffsetX=x;
	m_OffsetY=y;

	//delete by myf at 2013-8-1 保留所有空格，因为如果空格出现在第一行第一个字，那么下次通过record得到页面没有空格 
	//bool isFirstSpace;
	int topSpaceNumber(0);
// 	if (offset == 0)
// 	{
// 		isFirstSpace = true;
// 	}
// 	else
// 		isFirstSpace = false;

	if (y != m_curlineinfo.y)//新行
	{
		if (m_curlineinfo.words.size() > 0)
		{
			pwi->lines.push_back(m_curlineinfo);
		}	
		InitLineInfo();
		m_curlineinfo.x = x;
		m_curlineinfo.y = y;
	}

	CharCacheNode* pNode =NULL;
	int wordCount(0);
	bool lastwordIsEng(false);
	int countLength = m_OffsetX;
	size_t i;
	long t1;
	for (i = 0; i < str.size(); i++)
	{
		myWORD nowWchar=0; 
	    nowWchar=str[i];
		switch (nowWchar)
		{
		case 0x20:		 //L' '
			countLength += nhalfW;
			countLength += zijianju;
			wordCount++;
			break;			
		case 0x3000	:	//L'　'
			countLength += nFullW;
			countLength += zijianju;
			wordCount++;
			break;			
		case 0x9 :		//L'\t'
			countLength += nFullW * m_nTabSize;
			break;		
		case 0xa:		//L'\n'
			break;
		default:
			if (isEngChar(nowWchar))
			{
				lastwordIsEng = true;
			
				pEnglishFont->BuildCharCache(nowWchar);
				CharCacheNode* temp = pEnglishFont->GetCharCacheNode(nowWchar);
				countLength += temp->charAdvance;
				countLength -= zijianju;
			}
			else
			{
				lastwordIsEng = false;
				if (((nowWchar >> 8) & 0xff) == 0)//半角
				{
					countLength += nhalfW;
				}
				else 
				{
					int re = returnCharLeftRight(nowWchar);
					if (re == 1 || re == 2)
					{
						pNode = pFont->BuildChar(nowWchar);
						if (pNode == NULL)
						{
							countLength += nhalfW;
						}
						else
							countLength +=  pNode->x+pNode->w;
						if (pNode)
						{
							delete pNode;
							pNode = NULL;
						}
					}
					else
						countLength += nFullW;
				}
				wordCount++;
			}
			countLength += zijianju;
			break;
		}
		
	}
	if(lastwordIsEng)
		wordCount++;
	int shengyu = m_OutDevW - m_nMrginRight - countLength;
	int tianchong(0);
	int tianchongzhi(1);
	if (shengyu < 0 )
	{
		tianchongzhi = -1;
		/*shengyu *= -1;*/
	}
	
	if (flag && (wordCount - 1) > 0)
	{
		int a = shengyu / (wordCount - 1);
		zijianju += a;
		tianchong = abs(shengyu) % (wordCount - 1);
	}

	long t2;

//	t2 = GetTickCount();
	bool isEngFont;
	for (i = 0; i < str.size(); i++)
	{
		myWORD nowWchar=0; 
	    nowWchar=str[i];

		wordinfo word;
		word.titleType = content->titleType;
		word.textStyle = content->textStyle;
		word.lineIndex = pwi->lines.size();
		word.WordUnicod = nowWchar;
		switch (nowWchar)
		{
		case 0x20:		 //L' '
// 			if (isFirstSpace)
// 			{
// 				topSpaceNumber++;
// 			}
// 			else
// 			{
				word.width = nhalfW;
				word.height = pFont->GetFontHeight();
				word.x = m_OffsetX;
				word.y = m_OffsetY;
		//	}
			m_OffsetX += nhalfW;
			if (tianchong-- > 0)
			{
				m_OffsetX += tianchongzhi;
			}
			m_OffsetX += zijianju;
			break;			
		case 0x3000	:	//L'　'
// 			if (isFirstSpace)
// 			{
// 				topSpaceNumber++;
// 			}
// 			else
// 			{
				word.width = tempFullW;
				word.height = pFont->GetFontHeight();
				word.x = m_OffsetX;
				word.y = m_OffsetY;
		//	}
			m_OffsetX += tempFullW;
			if (tianchong-- > 0)
			{
				m_OffsetX += tianchongzhi;
			}
			m_OffsetX += zijianju;
			break;			
		case 0x9 :		//L'\t'
			word.x = m_OffsetX;
			word.y = m_OffsetY;
			m_OffsetX += tempFullW * m_nTabSize;
			word.width = tempFullW * m_nTabSize;
			word.height = pFont->GetFontHeight();
			if (tianchong-- > 0)
			{
				m_OffsetX += tianchongzhi;
			}
			break;		
		case 0xa:		//L'\n'
			m_nowlineEnd.x=m_OffsetX;
			m_nowlineEnd.y=m_OffsetY+tempFullW;
			goto exithah;
			break;
		default:
			//isFirstSpace = false;
			if (!isEngChar(nowWchar))
			{
				pNode = pFont->BuildChar(nowWchar);
// 				pFont->BuildCharCache(nowWchar);
// 				pNode = pFont->GetCharCacheNode(nowWchar);
				nFullW = tempFullW;
				if (tianchong-- > 0)
				{
					m_OffsetX += tianchongzhi;
				}
				isEngFont = false;
			}
			else
			{
				pEnglishFont->BuildCharCache(nowWchar);
				pNode = pEnglishFont->GetCharCacheNode(nowWchar);
				nFullW = nEFullW;
				isEngFont = true;
			}

			if (!pNode/* || !pNode->data*/)
			{	 
				m_OffsetX += nFullW;
				break;
				continue;			
			}else		
			{
				if (!bnewline)
				{
					bnewline=true;
					m_nowlineStart.x=m_OffsetX;
					m_nowlineStart.y=m_OffsetY+nFullW;
				}		

				if (((nowWchar >> 8) & 0xff) != 0 && i == str.size() - 1)
				{
					int devright = m_OutDevW - m_nMrginRight;
					int ww;
					int re = returnCharLeftRight(nowWchar);
					if ( re == 1 || re == 2)
					{
						ww =  (pNode->x+pNode->w);
						if (nowWchar == 0xff0c || nowWchar == 0x3002)
						{
							m_OffsetX -= pNode->x;
						}
					}
					else
					{
						ww = nFullW;
					}
					int avage = devright - (m_OffsetX +  ww);
					if (avage < (ww / 2))
					{
						m_OffsetX += avage;  
					}
				}
				else
				{
					if (returnCharLeftRight(nowWchar) == 1)
					{
						m_OffsetX -= pNode->x;
					}
// 					else if (returnCharLeftRight(nowWchar) == 2)
// 					{
// 						m_OffsetX += pNode->charAdvance - pNode->w;
// 					}
				}
			//	long t1 = GetTickCount();
				RenderChar(Image,pNode, m_OffsetX, m_OffsetY,returnCharLeftRight(nowWchar),isEngFont,tempFullW);
			//	long t2 = GetTickCount();
			//	TRACE("render char use time%ld\n",t2-t1);
				word.x = m_OffsetX;
				word.y = m_OffsetY + tempFullW-pNode->y -1;
				if (isEngChar(nowWchar))
				{
					m_OffsetX += pNode->charAdvance;
					word.height = pNode->h;
					word.width = pNode->w;
				}
				else
				{
					if (((nowWchar >> 8) & 0xff) == 0)
					{
						m_OffsetX += nhalfW;
						word.width = nhalfW;
					
					}
					else 
					{
						int re = returnCharLeftRight(nowWchar);
						if ( re == 1 || re == 2)
						{
							m_OffsetX += (pNode->x+pNode->w);
							word.width = (pNode->x+pNode->w);
							if (re == 1)
							{
								m_OffsetX += pNode->x;
							}
// 							else if (re == 2)
// 							{
// 								m_OffsetX -= (pNode->charAdvance - pNode->w);
// 							}
						}
						else
						{
							m_OffsetX += nFullW;
							word.width = nFullW;
						}
											
					}
					word.height = pNode->h;//nFullW;
// 					free(pNode->data);
// 					pNode->data = NULL;
					pNode->glyph = NULL;
 					delete pNode;
 					pNode = NULL;
					
					m_OffsetX += zijianju;
				}
				
			}
		} 

// 		if (!isFirstSpace)
// 		{
			word.contentID = contentID;
			m_curlineinfo.Width+=word.width;
			word.offset = i + offset;
		//	word.y = m_OffsetY;
			
			if (m_curlineinfo.Height < word.height)
			{
				m_curlineinfo.Height = word.height;
			}
			m_curlineinfo.words.push_back(word);
	//	}
		
		countWritechar++;
	}
//	t1 = GetTickCount();
//	TRACE("drawlien real draw use time:%ld\n",t1-t2);
	m_curlineinfo.RealwordNum += str.size();
exithah:
	if (countWritechar>0)
	{		
		m_nowlineEnd.x=m_OffsetX;
		m_nowlineEnd.y=m_OffsetY+nFullW;
		if((m_nFormat & DT_EX_UNDERLINE) && bnewline) 
		{
			if(m_nowlineEnd.y != m_nowlineStart.y)
			{
				m_nowlineEnd.y = m_nowlineStart.y;
			}
			DrawUnderLine(Image,m_nowlineStart,m_nowlineEnd,UnderlinePos);
		}	
	}
	m_OffsetX=OffsetX;
	m_OffsetY=OffsetY;
	return 0;
}
int  SZRenderSimulate::DrawString2(SZFTFont *pFont,SZFTFont *pEnglishFont,const std::wstring& str,int beginPos,int &addstuas,StyleSheet &css,bool iscssEnable)
{
	addstuas = 0;
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	int nFullW=pFont->GetFullWidthAdvanceCH();
	int nhalfW=pFont->GetHalfWidthAdvanceCH();
	int nEFullW=pEnglishFont->GetFullWidthAdvance();
	int nEhalfW=pEnglishFont->GetHalfWidthAdvance();
	int nEhalfH = pEnglishFont->GetEngHightAdvance();
	int UnderlinePos=pFont->GetUnderlinePos();
	myWORD nowWchar=0;
	int curlineWordCount(0);
	int curWordWidth;
	std::wstring strline = L"";
	int i;
	int beginoffset = beginPos;
	if ((m_iNowLine>0)&&(m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW)))
	{
		return 0;
	}

	for (i = 0; i < str.size();)
	{
#ifndef _WIN32
		if(g_IsDrawing)
		{
			pthread_mutex_lock(&g_ClacpageMutex);
			pthread_cond_wait(&g_cond,&g_ClacpageMutex);
		}
#endif
		nowWchar=str[i];
		if((beginPos>i)||(nowWchar== 0xd ))//L'\r'
		{ 
			i++;
			continue;
		}
		else if (nowWchar== 0xa)//L'\n'
		{			
			i++;
			curlineWordCount = 0;
			m_blineNew	= true;
			m_iNowLine++;
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			strline = L"";
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				addstuas=i;
#ifndef _WIN32
				//	if(g_IsDrawing)
				{
					pthread_mutex_unlock(&g_ClacpageMutex);
				}
#endif
				return 0;
			}
			continue;
		}
		else
		{
			
			if (nowWchar==0x20 )//L' '
			{
				OffsetX += nhalfW;
				OffsetX += m_nWordSpac;
				curWordWidth = nhalfW;
			}
			else if (nowWchar==0x3000)//L'　'
			{
				OffsetX += nFullW;
				OffsetX += m_nWordSpac;
				curWordWidth = nFullW;
			}
			else if (nowWchar==0x9)// L'\t'
			{
				OffsetX += nFullW * m_nTabSize;
				curWordWidth = nFullW * m_nTabSize;
			}
			else
			{
				curlineWordCount++;
				if (isEngChar(nowWchar))
				{
					pEnglishFont->BuildCharCache(nowWchar);
					CharCacheNode* temp = pEnglishFont->GetCharCacheNode(nowWchar);
					OffsetX += /*nEhalfW*/temp->charAdvance;
					curWordWidth = /*nEhalfW*/temp->charAdvance;
					OffsetX -= m_nWordSpac;
				}
				else
				{
					if (((nowWchar >> 8) & 0xff) == 0)
					{
						
						OffsetX += nhalfW;
						curWordWidth = nhalfW;		
					}
					else 
					{		
						//add at 2013-2-2
						int re = returnCharLeftRight(nowWchar);
						if (re == 1 || re == 2)
						{
							pFont->BuildCharCache(nowWchar);
							CharCacheNode* temp = pFont->GetCharCacheNode(nowWchar);
							if (temp)
							{
								OffsetX += temp->x + temp->w;
								curWordWidth = temp->x + temp->w;
								//delete temp;
							}
							else
							{
								OffsetX += nhalfW;
								curWordWidth = nhalfW;
							}
						}
						else
						{
							OffsetX += nFullW;
							curWordWidth = nFullW;
						}
					}
				}
				OffsetX += m_nWordSpac;
			}

			if (OffsetX > (m_OutDevW-m_nMrginRight))
			{
				int deleteCount(1);
				int overage = (m_OutDevW-m_nMrginRight) - (OffsetX - curWordWidth - m_nWordSpac);
				myWORD nextWchar;
				myWORD beforWchar;
				if (i < str.size()-1)
					nextWchar=str[i+1];
				else nextWchar=0;
				if(nowWchar == 0x20 || nowWchar == 0x3000)
				{
					i++;
					goto drawline;
				}
				switch(returnCharType(nowWchar))
				{
				case 1:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while (returnCharType(beforWchar) == 1)
						{
							deleteCount++;
							if (deleteCount > 4)
							{
								break;
							}
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								deleteCount -= 2;
								break;
							}
						}
						deleteCount++;
						if (returnCharType(beforWchar) == 3)
						{
							do 
							{
								if (i - deleteCount >= 0)
								{
									beforWchar = str[i - deleteCount];
								}
								else
									break;
								deleteCount++;
							} while (returnCharType(beforWchar) == 3);
						}
					}

					break;
				case 2:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 2)
						{
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								deleteCount--;
								break;
							}
						}
					}
					
					break;
				case 3:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 3)
						{
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else 
								break;
						}
					}
					if (deleteCount >= 20)
					{
						deleteCount = 2;
					}
					break;
				case 4:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 2)
						{
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								break;
							}
							
						}
					}
					break;
				}
				if((i - deleteCount) >= 0)
				{
					beforWchar = str[i-deleteCount];
					while(returnCharType(beforWchar) == 2)
					{
						deleteCount++;
						if (i - deleteCount >= 0)
						{
							beforWchar = str[i - deleteCount];
						}
						else
						{
							break;
						}
							
					}
					if (i - deleteCount + 1 == beginPos)
					{
						deleteCount = 1;
					}
				}
drawline:			OffsetX -= curWordWidth;
				curlineWordCount -= deleteCount;
				i -= (deleteCount - 1);
				m_OffsetX  = m_nMrginLeft;				
				m_OffsetY += nFullW+m_nVertSpac; 		
				OffsetX=m_OffsetX;
				OffsetY=m_OffsetY;
				strline = L"";
				if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
				{
					addstuas=i;
#ifndef _WIN32
					//	if(g_IsDrawing)
					{
						pthread_mutex_unlock(&g_ClacpageMutex);
					}
#endif
					return 0;
				}
			}
			else
			{
				strline += nowWchar;
				i++;
			}

		}
#ifndef _WIN32
		//	if(g_IsDrawing)
				{
					pthread_mutex_unlock(&g_ClacpageMutex);
				}
#endif
	}
	if (strline.size()>0)
	{
		m_nCharContentNum+=strline.size();
		strline = L"";
		m_OffsetX = OffsetX;				
		m_OffsetY = OffsetY; 
		if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
		{
			addstuas=i;
			return 0;
		}		
	}	
	return 1;

}
int  SZRenderSimulate::DrawString(SZFTFont *pFont,const std::wstring& str,int beginPos,int &addstuas,StyleSheet &css,bool iscssEnable)
{ 	
	addstuas=0;//content里的offset
	if (!pFont)
		return -1;
	//pFont->BuildStringCache(str); 
	int nowcharW=0;
	int nFullW=pFont->GetFullWidthAdvanceCH();
	int nhalfW=pFont->GetHalfWidthAdvanceCH();
	int UnderlinePos=pFont->GetUnderlinePos();
	
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	int nextWchar=0;
	
	//CharCacheNode* pNode =NULL;
	std::wstring  strline= L"";		
	int rtretry=0;
	if ((m_iNowLine>0)&&(m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW)))
	{
		return 0;
	}
	size_t i;
	for (i = 0; i < str.length();)
	{	
		myWORD nowWchar=0; 
		nowWchar=str[i];

		if((beginPos>i)||(nowWchar== 0xd ))//L'\r'
		{ 
			i++;
			continue;
		}
	
		if (nowWchar== 0xa)//L'\n'
		{			
			i++;
			if (strline.length()>0)		 
				m_nCharContentNum+=strline.length();
			m_blineNew	= true;
			m_iNowLine++;
			strline= L"";
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				addstuas=i;
				return 0;
			}
			continue;
		}

		if (i < str.length()-1)
				nextWchar=str[i+1];
		else nextWchar=0;	
		
		rtretry=RetryDrawLine(pFont,strline,nowWchar,nextWchar,nFullW,nhalfW,OffsetX,OffsetY,nowcharW);
		if (rtretry<0)
			return rtretry;

  		if (rtretry==0)//不添加在该行，换行
		{		
			if (strline.length()>0)		 
				m_nCharContentNum+=strline.length();
			m_blineNew	   = true;
			m_iNowLine++;
			strline= L"";
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				addstuas=i;
				return 0;
			}	
		}		
		else
		{
			if (m_blineNew)
				m_blineNew	= false;				
			i+=rtretry;
		}	
	}  

	if (strline.length()>0)
	{
		m_nCharContentNum+=strline.length();
		strline= L"";	
		m_OffsetX = OffsetX;				
		m_OffsetY = OffsetY; 
		if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
		{
			addstuas=i;
			return 0;
		}		
	}	

	return 1;
}
int	SZRenderSimulate::DrawString2(SZFTFont *pFont,SZFTFont *pEnglishFont,const Vt_myString& str,int beginPos,int &addstuas,StyleSheet &css,bool iscssEnable)
{
	addstuas = 0;
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	int nFullW=pFont->GetFullWidthAdvanceCH();
	int nhalfW=pFont->GetHalfWidthAdvanceCH();
	int nEFullW=pEnglishFont->GetFullWidthAdvance();
	int nEhalfW=pEnglishFont->GetHalfWidthAdvance();
	int UnderlinePos=pFont->GetUnderlinePos();
	myWORD nowWchar=0;
	int curlineWordCount(0);
	int curWordWidth;
//	int length1,length2, xxx;
	Vt_myString  strline;
	int i;
	int beginoffset = beginPos;
	if ((m_iNowLine>0)&&(m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW)))
	{
		return 0;
	}
	for (i = 0; i < str.size();)
	{
#ifndef _WIN32
		if(g_IsDrawing)
		{
			pthread_mutex_lock(&g_ClacpageMutex);
			pthread_cond_wait(&g_cond,&g_ClacpageMutex);
		}
#endif
		
		nowWchar=str[i];
		if((beginPos>i)||(nowWchar== 0xd ))//L'\r'
		{ 
			i++;
			continue;
		}
		else if (nowWchar== 0xa)//L'\n'
		{			
			i++;
			strline.clear();
			m_blineNew	= true;
			m_iNowLine++;
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				addstuas=i;
#ifndef _WIN32
				//	if(g_IsDrawing)
				{
					pthread_mutex_unlock(&g_ClacpageMutex);
				}
#endif
				return 0;
			}
			continue;
		}
		else
		{
			if (nowWchar==0x20 )//L' '
			{
				OffsetX += nhalfW;
				OffsetX += m_nWordSpac;
				curWordWidth = nhalfW;
			}
			else if (nowWchar==0x3000)//L'　'
			{
				OffsetX += nFullW;
				OffsetX += m_nWordSpac;
				curWordWidth = nFullW;
			}
			else if (nowWchar==0x9)// L'\t'
			{
				OffsetX += nFullW * m_nTabSize;
				curWordWidth = nFullW * m_nTabSize;
			}
			else
			{
				curlineWordCount++;
				if (isEngChar(nowWchar))
				{
					pEnglishFont->BuildCharCache(nowWchar);
					CharCacheNode* temp = pEnglishFont->GetCharCacheNode(nowWchar);

					OffsetX += /*nEhalfW*/temp->charAdvance;;
					OffsetX -= m_nWordSpac;
					curWordWidth = /*nEhalfW*/temp->charAdvance;
				}
				else
				{
					if (((nowWchar >> 8) & 0xff) == 0)
					{
						
						OffsetX += nhalfW;
						curWordWidth = nhalfW;
					}
					else 
					{
						
						//add at 2013-2-2
						int re = returnCharLeftRight(nowWchar);
						if (re == 1 || re == 2)
						{
							pFont->BuildCharCache(nowWchar);
							CharCacheNode* temp = pFont->GetCharCacheNode(nowWchar);
							if (temp)
							{
								OffsetX += temp->x + temp->w;
								curWordWidth = temp->x + temp->w;
								//delete temp;
							}
							else
							{
								OffsetX += nhalfW;
								curWordWidth = nhalfW;
							}
						}
						else
						{
							OffsetX += nFullW;
							curWordWidth = nFullW;
						}
						
						
					}
				}
			
				OffsetX += m_nWordSpac;
			}

			if (OffsetX > (m_OutDevW-m_nMrginRight))
			{
				int deleteCount(1);
				
				int overage = (m_OutDevW-m_nMrginRight) - (OffsetX - curWordWidth - m_nWordSpac);
				myWORD nextWchar;
				myWORD beforWchar;
				if (i < str.size()-1)
					nextWchar=str[i+1];
				else nextWchar=0;
				if(nowWchar == 0x20 || nowWchar == 0x3000)
				{
					i++;
					goto drawline;
				}
				switch(returnCharType(nowWchar))
				{
				case 1:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 1)
						{
							deleteCount++;
							if (deleteCount > 4)
							{
								break;
							}
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								deleteCount -= 2;
								break;
							}
							
						}
						deleteCount++;
						if (returnCharType(beforWchar) == 3)
						{
							do 
							{
								if (i - deleteCount >= 0)
								{
									beforWchar = str[i - deleteCount];
								}
								else
									break;
								deleteCount++;
							} while (returnCharType(beforWchar) == 3);
						}
					}
					break;
				case 2:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 2)
						{
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								deleteCount--;
								break;
							}
						}
					}	
					break;
				case 3:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 3)
						{
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else 
								break;
						}
					}
					if (deleteCount >= 20)
					{
						deleteCount = 2;
					}
					break;
				case 4:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 2)
						{
							overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								break;
							}
							
						}
					}
					break;
				}
				if((i - deleteCount) >= 0)
				{
					beforWchar = str[i-deleteCount];
					while(returnCharType(beforWchar) == 2)
					{
						deleteCount++;
						if (i - deleteCount >= 0)
						{
							beforWchar = str[i - deleteCount];
						}
						else
						{
							break;
						}
							
					}
					if (i -deleteCount + 1 == beginPos)
					{
						deleteCount = 1;
					}
				}
drawline:				OffsetX -= curWordWidth;
				curlineWordCount -= deleteCount;
				i -= (deleteCount - 1);
					
				m_OffsetX  = m_nMrginLeft;				
				m_OffsetY += (nFullW+m_nVertSpac); 		
				OffsetX=m_OffsetX;
				OffsetY=m_OffsetY;
				strline.clear();
				
				if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
				{
					addstuas=i;	
#ifndef _WIN32
					//	if(g_IsDrawing)
					{
						pthread_mutex_unlock(&g_ClacpageMutex);
					}
#endif
					return 0;
				}
				
			}
			else
			{
				strline.push_back(nowWchar);
				i++;
			}

		}
#ifndef _WIN32
	//	if(g_IsDrawing)
		{
			pthread_mutex_unlock(&g_ClacpageMutex);
		}
#endif
	}
	if (strline.size()>0)
	{
		strline.clear();
		m_OffsetX = OffsetX;				
		m_OffsetY = OffsetY; 
		if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
		{
			addstuas=i;
			return 0;
		}		
	}	
	return 1;
}

int  SZRenderSimulate::DrawString(SZFTFont *pFont,const Vt_myString& str,int beginPos,int &addstuas,StyleSheet &css,bool iscssEnable)
{ 	
	addstuas=0;
	if (!pFont)
		return -1;
//	pFont->BuildStringCache(str); 
	int nowcharW=0;
	int nFullW=pFont->GetFullWidthAdvanceCH();
	int nhalfW=pFont->GetHalfWidthAdvanceCH();
	int UnderlinePos=pFont->GetUnderlinePos();
	
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	int nextWchar=0;
	
	CharCacheNode* pNode =NULL;
	Vt_myString strline;
	strline.clear();
	int rtretry=0;
	if ((m_iNowLine>0)&&(m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW)))
	{
		return 0;
	}
	size_t i;
	for (i = 0; i < str.size();)
	{	
		myWORD nowWchar=0; 
		nowWchar=str[i];
		
		if((beginPos>i)||(nowWchar== 0xd ))//L'\r'
		{ 
			i++;
			continue;
		}
		
		if (nowWchar== 0xa)//L'\n'
		{			
			i++;
			if (strline.size()>0)		 
				m_nCharContentNum+=strline.size();
			m_blineNew	= true;
			m_iNowLine++;
			strline.clear();
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				addstuas=i;
				return 0;
			}
			continue;
		}
		
		if (i < str.size()-1)
			nextWchar=str[i+1];
		else nextWchar=0;	
		
		rtretry=RetryDrawLine(pFont,strline,nowWchar,nextWchar,nFullW,nhalfW,OffsetX,OffsetY,nowcharW);
		if (rtretry<0)
			return rtretry;
		
		if (rtretry==0)
		{			 
			m_blineNew	   = true;
			m_iNowLine++;
			if (strline.size()>0)		 
				m_nCharContentNum+=strline.size();
			strline.clear();
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				addstuas=i;
				return 0;
			}	
		}		
		else
		{
			if (m_blineNew)
				m_blineNew	= false;				
			i+=rtretry;
		}	
	}  
	
	if (strline.size()>0)
	{
		m_nCharContentNum+=strline.size();
		strline.clear();
		m_OffsetX = OffsetX;				
		m_OffsetY = OffsetY; 
		if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
		{
			addstuas=i;
			return 0;
		}		
	}	
	
	return 1;
}
int  SZRender::DrawString2(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const std::wstring& str,int beginPos,int &addstuas,
						  StyleSheet &css,bool iscssEnable,int contentID,pageWordinfo* pwi,Vt_curPageInterLinks* Cpil,string url,Content* content)
{
	addstuas = 0;
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	int nFullW=pFont->GetFullWidthAdvanceCH();
	int nhalfW=pFont->GetHalfWidthAdvanceCH();
	int nEFullW=pEnglishFont->GetFullWidthAdvance();
	int nEhalfW=pEnglishFont->GetHalfWidthAdvance();
	int UnderlinePos=pFont->GetUnderlinePos();
	myWORD nowWchar=0;
	int curlineWordCount(0);
	int curWordWidth;
	int length1,length2, xxx;
	std::wstring  strline= L"";
	int i;
	int beginoffset = beginPos;
	if ((m_iNowLine>0)&&(m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW)))
	{
		return 0;
	}

	bool islink = (url!=""?true:false);
	interlink il;
	for (i = beginPos; i < str.size();)
	{
		nowWchar=str[i];
		if((beginPos>i)||(nowWchar== 0xd ))//L'\r'
		{ 
			i++;
			continue;
		}
		else if (nowWchar== 0xa)//L'\n'
		{			
			i++;
			if (strline.length()>0)
			{
				if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
				{ 					
					length1=OffsetX-m_OffsetX;
					length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
					xxx=(length2-length1)/2+m_nMrginLeft;
					DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi,content);					
					il.lefttop_x = xxx;
				}else
				{
					DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi,content);
					il.lefttop_x = m_OffsetX;
				}	
				if (islink)
				{
					il.lefttop_y = m_OffsetY;
					il.rightbottom_x = OffsetX;
					il.rightbottom_y = OffsetY + nFullW;
					memset(il.url,0,256);
					memcpy(il.url,url.c_str(),url.length());
					Cpil->push_back(il);
				}
				beginoffset += m_curlineinfo.RealwordNum;
			}
			curlineWordCount = 0;
			m_blineNew	   = true;
			m_iNowLine++;
			strline= L"";
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += (nFullW+m_nVertSpac); 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				if (m_curlineinfo.words.size() > 0)
				{
					pwi->lines.push_back(m_curlineinfo);
					InitLineInfo();
				}
				addstuas=i;
				return 0;
			}
			
			continue;
		}
		else
		{
			if (nowWchar==0x20 )//L' '
			{
				OffsetX += nhalfW;
				OffsetX -= m_nWordSpac;
				curWordWidth = nhalfW;
			}
			else if (nowWchar==0x3000)//L'　'
			{
				OffsetX += nFullW;
				OffsetX -= m_nWordSpac;
				curWordWidth = nFullW;
			}
			else if (nowWchar==0x9)// L'\t'
			{
				OffsetX += nFullW * m_nTabSize;
				curWordWidth = nFullW * m_nTabSize;
			}
			else
			{
				curlineWordCount++;
				if (isEngChar(nowWchar))
				{
					pEnglishFont->BuildCharCache(nowWchar);
					CharCacheNode* temp = pEnglishFont->GetCharCacheNode(nowWchar);
					OffsetX += /*nEhalfW*/temp->charAdvance;
					OffsetX -= m_nWordSpac;
					curWordWidth = /*nEhalfW*/temp->charAdvance;
				}
				else
				{
					if (((nowWchar >> 8) & 0xff) == 0)
					{
						
						OffsetX += nhalfW;
						curWordWidth = nhalfW;
					}
					else 
					{
						
						//add at 2013-2-2
						int re = returnCharLeftRight(nowWchar);
						if (re == 1 || re ==2)
						{
							pFont->BuildCharCache(nowWchar);
							CharCacheNode* pNode = pFont->GetCharCacheNode(nowWchar);
							if (pNode == NULL)
							{
								OffsetX += nhalfW;
								curWordWidth = nhalfW;
							}
							else
							{
								OffsetX += pNode->x+pNode->w;
								curWordWidth = pNode->x+pNode->w;
							}
							
						}
						else
						{
							OffsetX += nFullW;
							curWordWidth = nFullW;
						}
						
					}
				}
			
				OffsetX += m_nWordSpac;
			}

			if (OffsetX > (m_OutDevW-m_nMrginRight))
			{
			
				int deleteCount(1);
				int overage = (m_OutDevW-m_nMrginRight) - (OffsetX - curWordWidth - m_nWordSpac);
				myWORD beforWchar;
				if(nowWchar == 0x20 || nowWchar == 0x3000)
				{
					i++;
					goto drawline;
				}
				switch(returnCharType(nowWchar))
				{
				case 1:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 1)
						{
							overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
							overage += m_nWordSpac;
							deleteCount++;
							if (deleteCount > 4)
							{
								break;
							}
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								deleteCount -= 2;
								break;
							}
							
						}
						deleteCount++;
						if (returnCharType(beforWchar) == 3)
						{
							do 
							{
								if (i - deleteCount >= 0)
								{
									beforWchar = str[i - deleteCount];
									if (((beforWchar >> 8) & 0xff) == 0)
									{
										overage += nhalfW;			
									}
									else 
									{
										overage += nFullW;
									}
								}
								else
									break;
								deleteCount++;
							} while (returnCharType(beforWchar) == 3);
						}
					}
					break;
				case 2:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 2)
						{
							overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
							overage += m_nWordSpac;
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								deleteCount--;
								break;
							}
						}
					}
					
					break;
				case 3:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 3)
						{
							overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else 
								break;
						}
					}
					if (deleteCount >= 20)
					{
						deleteCount = 2;
						overage = (m_OutDevW-m_nMrginRight) - (OffsetX - curWordWidth - m_nWordSpac);
					}
					break;
				case 4:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 2)
						{
							overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								break;
							}
							
						}
					}
					break;
				}
				if((i - deleteCount) >= 0)
				{
					beforWchar = str[i-deleteCount];
					while(returnCharType(beforWchar) == 2)
					{
						overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
						overage += m_nWordSpac;
						deleteCount++;
						if (i - deleteCount >= 0)
						{
							beforWchar = str[i - deleteCount];
						}
						else
						{
							break;
						}
							
					}
					if (i - deleteCount + 1 == beginPos)
					{
						deleteCount = 1;
					}
				}
drawline:			OffsetX -= curWordWidth;
				curlineWordCount -= deleteCount;
				i -= (deleteCount - 1);
				
				if (strline.size() > 0)
				{
					strline.erase(strline.end() - deleteCount + 1,strline.end());
				}

				float f_jianju = (float)overage / (float)(curlineWordCount-1);
				int i_jianju = int(f_jianju);
				if (f_jianju > (i_jianju + 0.5))
				{
					i_jianju++;
				}
				if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
				{ 					
					length1=OffsetX-m_OffsetX;
					length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
					xxx=(length2-length1)/2+m_nMrginLeft;
					DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac + i_jianju,contentID,beginoffset,pwi,content,true);					
					il.lefttop_x = xxx;
				}else
				{
					DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac + i_jianju,contentID,beginoffset,pwi,content,true);
					il.lefttop_x = m_OffsetX;
				}	
				if (islink)
				{
					il.lefttop_y = m_OffsetY;
					il.rightbottom_x = OffsetX;
					il.rightbottom_y = OffsetY + nFullW;
					memset(il.url,0,256);
					memcpy(il.url,url.c_str(),url.length());
					Cpil->push_back(il);
				}
				strline= L"";
				beginoffset += m_curlineinfo.RealwordNum;	
				m_OffsetX  = m_nMrginLeft;				
				m_OffsetY += (nFullW+m_nVertSpac); 		
				OffsetX=m_OffsetX;
				OffsetY=m_OffsetY;
				curlineWordCount = 0;

				if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
				{
					addstuas=i;
					if (m_curlineinfo.words.size() > 0)
					{
						pwi->lines.push_back(m_curlineinfo);
						InitLineInfo();
					}					
					return 0;
				}
				
			}
			else
			{
				strline+=nowWchar;
				i++;
			}

		}

	}
	if (strline.length()>0)
	{
		m_nCharContentNum+=strline.length();			
		if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
		{ 					
			length1=OffsetX-m_OffsetX;
			length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
			xxx=(length2-length1)/2+m_nMrginLeft;
			DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi,content);					
		}else
		{
			DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi,content);
			il.lefttop_x = m_OffsetX;
		}	
		if (islink)
		{
			il.lefttop_y = m_OffsetY;
			il.rightbottom_x = OffsetX;
			il.rightbottom_y = OffsetY + nFullW;
			memset(il.url,0,256);
			memcpy(il.url,url.c_str(),url.length());
			Cpil->push_back(il);
		}
		strline= L"";
		beginoffset += m_curlineinfo.RealwordNum;
		m_OffsetX = OffsetX;				
		m_OffsetY = OffsetY; 

		if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
		{
			addstuas=i;
			if (m_curlineinfo.words.size() > 0)
			{
				pwi->lines.push_back(m_curlineinfo);
				InitLineInfo();
			}			
			return 0;
		}	
		
	}
	return 1;
}
int  SZRender::DrawString(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const std::wstring& str,int beginPos,int &addstuas,
						  StyleSheet &css,bool iscssEnable,int contentID,pageWordinfo* pwi)
{
	addstuas=0;
	if (!pFont)
		return -1;
	int nowcharW=0;
	int nFullW=pFont->GetFullWidthAdvanceCH();
	int nhalfW=pFont->GetHalfWidthAdvanceCH();
	int UnderlinePos=pFont->GetUnderlinePos();
	int length1,length2, xxx;
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	int nextWchar=0;
	std::wstring  strline= L"";		
	int rtretry=0;

	int beginoffset = beginPos;
	if ((m_iNowLine>0)&&(m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW)))
	{
		return 0;
	}	
	size_t i;
	for (i = 0; i < str.length(); )
	{	
		myWORD nowWchar=0;  
		nowWchar=str[i];
 
		if((beginPos>i)||(nowWchar== 0xd ))
		{ 
			i++;
			continue;
		}
		
		if (nowWchar== 0xa)
		{
			i++;
			if (strline.length()>0)
			{
				if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
				{ 					
					length1=OffsetX-m_OffsetX;
					length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
					xxx=(length2-length1)/2+m_nMrginLeft;
					//DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);					
				}else
				{
				//	DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);
				}	
				beginoffset += m_curlineinfo.RealwordNum;
			}
			
			m_blineNew	   = true;
			m_iNowLine++;
			strline= L"";
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				if (m_curlineinfo.words.size() > 0)
				{
					pwi->lines.push_back(m_curlineinfo);
					InitLineInfo();
				}
				addstuas=i;
				return 0;
			}
			continue;
		}

		if (i < str.length()-1)
		{
			nextWchar=str[i+1];
		}			 
		else nextWchar=0;
					
	
		rtretry=RetryDrawLine(pFont,strline,nowWchar,nextWchar,nFullW,nhalfW,OffsetX,OffsetY,nowcharW);		
		if (rtretry<0)
			return rtretry;
	 
		if (rtretry==0)
		{	
			if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
			{ 					
				length1=OffsetX-m_OffsetX;
				length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
				xxx=(length2-length1)/2+m_nMrginLeft;
				//DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);					
			}else
			{
			//	DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);
			}		
			beginoffset += m_curlineinfo.RealwordNum;			
			m_blineNew	   = true;
			m_iNowLine++;
			strline= L"";
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				addstuas=i;

				if (m_curlineinfo.words.size() > 0)
				{
					pwi->lines.push_back(m_curlineinfo);
					InitLineInfo();
				}

				return 0;
			}			 
		}else
		{
			if (m_blineNew)
				m_blineNew	= false;	
			i+=rtretry;
		}		
	}

	if (strline.length()>0)
	{
		if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
		{ 					
			length1=OffsetX-m_OffsetX;
			length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
			xxx=(length2-length1)/2+m_nMrginLeft;
			//DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);					
		}else
		{
			//DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);
		}	
		beginoffset += m_curlineinfo.RealwordNum;
		strline= L"";	
		m_OffsetX = OffsetX;				
		m_OffsetY = OffsetY; 
		if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
		{
			addstuas=i;

			if (m_curlineinfo.words.size() > 0)
			{
				pwi->lines.push_back(m_curlineinfo);
				InitLineInfo();
			}
			return 0;
		}		
	}
	
	
	return 1;
}
int  SZRender::DrawString2(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const Vt_myString& str,int beginPos,int &addstuas,
						  StyleSheet &css,bool iscssEnable,int contentID,pageWordinfo* pwi,Vt_curPageInterLinks* Cpil,string url,Content* content)
{
	addstuas = 0;
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	int nFullW=pFont->GetFullWidthAdvanceCH();
	int nhalfW=pFont->GetHalfWidthAdvanceCH();
	int nEFullW=pEnglishFont->GetFullWidthAdvance();
	int nEhalfW=pEnglishFont->GetHalfWidthAdvance();
	int UnderlinePos=pFont->GetUnderlinePos();
	myWORD nowWchar=0;
	int curlineWordCount(0);
	int curWordWidth;
	int length1,length2, xxx;
	Vt_myString  strline;
	strline.clear();
	long t1,t2;
	int i;
	int beginoffset = beginPos;
	if ((m_iNowLine>0)&&(m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW)))
	{
		return 0;
	}
	
bool islink = (url!=""?true:false);
	interlink il;
	for (i = 0; i < str.size();)
	{
		nowWchar=str[i];
		if (nowWchar == 0x201c)
		{
			nowWchar = str[i];
		}
		if((beginPos>i)||(nowWchar== 0xd ))//L'\r'
		{ 
			i++;
			continue;
		}
		else if (nowWchar== 0xa)//L'\n'
		{			
			i++;
			if (strline.size()>0)
			{
				if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
				{ 					
					length1=OffsetX-m_OffsetX;
					length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
					xxx=(length2-length1)/2+m_nMrginLeft;
				//	t1 = GetTickCount();
					DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi,content);	
				//	t2 = GetTickCount();
				//	TRACE("DrawLine %d use time:%ld\n",m_OffsetY,t2-t1);
					il.lefttop_x = xxx;
				}else
				{
				//	t1 = GetTickCount();
					DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi,content);
			//		t2 = GetTickCount();
				//	TRACE("DrawLine %d use time:%ld\n",m_OffsetY,t2-t1);
					il.lefttop_x = m_OffsetX;
				}	
				if (islink)
				{
					il.lefttop_y = m_OffsetY;
					il.rightbottom_x = OffsetX;
					il.rightbottom_y = OffsetY + nFullW;
					memset(il.url,0,256);
					memcpy(il.url,url.c_str(),url.length());
					Cpil->push_back(il);
				}
				beginoffset += m_curlineinfo.RealwordNum;
			}
			curlineWordCount = 0;
			m_blineNew	   = true;
			m_iNowLine++;
			strline.clear();
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += (nFullW+m_nVertSpac); 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				if (m_curlineinfo.words.size() > 0)
				{
					pwi->lines.push_back(m_curlineinfo);
					InitLineInfo();
				}
				addstuas=i;
				return 0;
			}
			
			continue;
		}
		else
		{
			if (nowWchar==0x20 )//L' '
			{
				OffsetX += nhalfW;
				OffsetX += m_nWordSpac;
				curWordWidth = nhalfW;
			}
			else if (nowWchar==0x3000)//L'　'
			{
				OffsetX += nFullW;
				OffsetX += m_nWordSpac;
				curWordWidth = nFullW;
			}
			else if (nowWchar==0x9)// L'\t'
			{
				OffsetX += nFullW * m_nTabSize;
				curWordWidth = nFullW * m_nTabSize;
			}
			else
			{
				curlineWordCount++;
				if (isEngChar(nowWchar))
				{
					pEnglishFont->BuildCharCache(nowWchar);
					CharCacheNode* temp = pEnglishFont->GetCharCacheNode(nowWchar);

					OffsetX += /*nEhalfW*/temp->charAdvance;
					OffsetX -= m_nWordSpac;
					curWordWidth = /*nEhalfW*/temp->charAdvance;
				}
				else
				{
					if (((nowWchar >> 8) & 0xff) == 0)
					{
						
						OffsetX += nhalfW;
						curWordWidth = nhalfW;
					}
					else 
					{
						//add at 2013-2-2
						int re = returnCharLeftRight(nowWchar);
						if (re == 1 || re ==2)
						{
							pFont->BuildCharCache(nowWchar);
							CharCacheNode* pNode = pFont->GetCharCacheNode(nowWchar);
							if (pNode == NULL)
							{
								OffsetX += nhalfW;
								curWordWidth = nhalfW;
							}
							else
							{
								OffsetX += pNode->x+pNode->w;
								curWordWidth = pNode->x+pNode->w;
							}
						
						}
						else
						{
							OffsetX += nFullW;
							curWordWidth = nFullW;
						}
					}
				}
			
				OffsetX += m_nWordSpac;
			}

			if (OffsetX > (m_OutDevW-m_nMrginRight))
			{
				int deleteCount(1);
				
				int overage = (m_OutDevW-m_nMrginRight) - (OffsetX - curWordWidth - m_nWordSpac);
				myWORD nextWchar;
				myWORD beforWchar;
				if (i < str.size()-1)
					nextWchar=str[i+1];
				else nextWchar=0;
				if(nowWchar == 0x20 || nowWchar == 0x3000)
				{
					i++;
					goto drawline;
				}
				switch(returnCharType(nowWchar))
				{
				case 1:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 1)
						{
							overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
							overage += m_nWordSpac;
							deleteCount++;
							if (deleteCount > 4)
							{
								break;
							}
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								deleteCount -= 2;
								break;
							}
							
						}
						deleteCount++;
						if (returnCharType(beforWchar) == 3)
						{
							do 
							{
								if (i - deleteCount >= 0)
								{
									beforWchar = str[i - deleteCount];
									if (((beforWchar >> 8) & 0xff) == 0)
									{
										overage += nhalfW;			
									}
									else 
									{
										overage += nFullW;
									}
								}
								else
									break;
								deleteCount++;
							} while (returnCharType(beforWchar) == 3);
						}
					}
					break;
				case 2:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 2)
						{
							overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
							overage += m_nWordSpac;
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								deleteCount--;
								break;
							}
						}
					}	
					break;
				case 3:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 3)
						{
							overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else 
								break;
						}
					}
					if (deleteCount >= 20)
					{
						deleteCount = 2;
						overage = (m_OutDevW-m_nMrginRight) - (OffsetX - curWordWidth - m_nWordSpac);
					}
					break;
				case 4:
					if (i - deleteCount >= 0)
					{
						beforWchar = str[i - deleteCount];
						while(returnCharType(beforWchar) == 2)
						{
							overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
							deleteCount++;
							if (i - deleteCount >= 0)
							{
								beforWchar = str[i - deleteCount];
							}
							else
							{
								break;
							}
							
						}
					}
					break;
				}
				if((i - deleteCount) >= 0)
				{
					beforWchar = str[i-deleteCount];
					while(returnCharType(beforWchar) == 2)
					{
						overage += returnCharWidth(beforWchar,nFullW,nhalfW,pEnglishFont,pFont);
						overage += m_nWordSpac;
						deleteCount++;
						if (i - deleteCount >= 0)
						{
							beforWchar = str[i - deleteCount];
						}
						else
						{
							break;
						}
						
					}
					if (i - deleteCount + 1 == beginPos)
					{
						deleteCount = 1;
					}
				}
drawline:				OffsetX -= curWordWidth;
				curlineWordCount -= deleteCount;
				i -= (deleteCount - 1);
				if (strline.size() > 0)
				{
					strline.erase(strline.end() - deleteCount + 1,strline.end());
				}
				
				float f_jianju = (float)overage / (float)(curlineWordCount-1);
				int i_jianju = int(f_jianju);
				if (f_jianju > (i_jianju + 0.5))
				{
					i_jianju++;
				}
				if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
				{ 					
					length1=OffsetX-m_OffsetX;
					length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
					xxx=(length2-length1)/2+m_nMrginLeft;
				//	t1 = GetTickCount();
					DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac + i_jianju,contentID,beginoffset,pwi,content,true);
				//	t2 = GetTickCount();
				//	TRACE("DrawLine %d use time:%ld\n",m_OffsetY,t2-t1);
					il.lefttop_x = xxx;
				}else
				{

				//	t1 = GetTickCount();
					DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac + i_jianju,contentID,beginoffset,pwi,content,true);
				//	t2 = GetTickCount();
				//	TRACE("DrawLine %d use time:%ld\n",m_OffsetY,t2-t1);
					il.lefttop_x = m_OffsetX;
				}
				if(islink)
				{
					il.lefttop_y = m_OffsetY;
					il.rightbottom_x = OffsetX;
					il.rightbottom_y = OffsetY + nFullW;
					memset(il.url,0,256);
					memcpy(il.url,url.c_str(),url.length());
					Cpil->push_back(il);
				}
				strline.clear();
				beginoffset += m_curlineinfo.RealwordNum;	
				m_OffsetX  = m_nMrginLeft;				
				m_OffsetY += (nFullW+m_nVertSpac); 		
				OffsetX=m_OffsetX;
				OffsetY=m_OffsetY;
				curlineWordCount = 0;
				
				if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
				{
					addstuas=i;
					if (m_curlineinfo.words.size() > 0)
					{
						pwi->lines.push_back(m_curlineinfo);
						InitLineInfo();
					}					
					return 0;
				}
				
			}
			else
			{
				strline.push_back(nowWchar);
				i++;
			}

		}

	}
	if (strline.size()>0)
	{
		m_nCharContentNum+=strline.size();
			
		if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
		{ 					
			length1=OffsetX-m_OffsetX;
			length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
			xxx=(length2-length1)/2+m_nMrginLeft;
		//	t1 = GetTickCount();
			DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi,content);	
		//	/t2 = GetTickCount();
		//			TRACE("DrawLine %d use time:%ld\n",m_OffsetY,t2-t1);
			il.lefttop_x = xxx;
		}else
		{
		//	t1 = GetTickCount();
			DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi,content);
		//	t2 = GetTickCount();
		//			TRACE("DrawLine %d use time:%ld\n",m_OffsetY,t2-t1);
			il.lefttop_x = m_OffsetX;
		}	
		if (islink)
		{
			il.lefttop_y = m_OffsetY;
			il.rightbottom_x = OffsetX;
			il.rightbottom_y = OffsetY + nFullW;
			memset(il.url,0,256);
			memcpy(il.url,url.c_str(),url.length());
			Cpil->push_back(il);
		}
		strline.clear();	
		beginoffset += m_curlineinfo.RealwordNum;
		m_OffsetX = OffsetX;				
		m_OffsetY = OffsetY; 
		if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
		{
			addstuas=i;
			if (m_curlineinfo.words.size() > 0)
			{
				pwi->lines.push_back(m_curlineinfo);
				InitLineInfo();
			}
			return 0;
		}
				
	}
	return 1;
}
int  SZRender::DrawString(SZImageBit24  &Image,SZFTFont *pFont,SZFTFont *pEnglishFont,const Vt_myString& str,int beginPos,int &addstuas,
						  StyleSheet &css,bool iscssEnable,int contentID,pageWordinfo* pwi)
{
	addstuas=0;
	if (!pFont)
		return -1;

//	pFont->BuildStringCache(str); 
	int nowcharW=0;
	int nFullW=pFont->GetFullWidthAdvanceCH();
	int nhalfW=pFont->GetHalfWidthAdvanceCH();
	int UnderlinePos=pFont->GetUnderlinePos();
	int length1,length2, xxx;
	int OffsetX=m_OffsetX;
	int OffsetY=m_OffsetY;
	int nextWchar=0;

//	CharCacheNode* pNode =NULL;
 	Vt_myString strline;
	strline.clear();
	int rtretry=0;

	int beginoffset = beginPos;

	if ((m_iNowLine>0)&&(m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW)))
	{
		return 0;
	}
	size_t i = 0;	
	for (i = 0; i < str.size(); )
	{	
        myWORD nowWchar=str[i];
		if((beginPos>i)||(nowWchar== 0xd ))//L'\r' //if((beginPos>i)||(str[i]== L'\r'))
		{ 
			i++;
			continue;
		}
		
		if (nowWchar== 0xa)//L'\n'if (str[i]== L'\n')
		{
			i++;
			if (strline.size()>0)
			{
				if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
				{ 					
					length1=OffsetX-m_OffsetX;
					length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
					xxx=(length2-length1)/2+m_nMrginLeft;
				//	DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);					
				}else
				{
				//	DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);
				}				
			}
			beginoffset += m_curlineinfo.RealwordNum;
			m_blineNew	   = true;
			m_iNowLine++;
			strline.clear();
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				if (m_curlineinfo.words.size() > 0)
				{
					pwi->lines.push_back(m_curlineinfo);
					InitLineInfo();
				}
				addstuas=i;
				return 0;
			}
			continue;
		}

		if (i < str.size()-1)
		{
             nextWchar=str[i+1];
		}			 
		else nextWchar=0;
					
	
		rtretry=RetryDrawLine(pFont,strline,nowWchar,nextWchar,nFullW,nhalfW,OffsetX,OffsetY,nowcharW);		
		if (rtretry<0)
			return rtretry;
	 
		if (rtretry==0)
		{	
			if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
			{ 					
				length1=OffsetX-m_OffsetX;
				length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
				xxx=(length2-length1)/2+m_nMrginLeft;
				//DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);					
			}else
			{
				//DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);
			}		
			beginoffset += m_curlineinfo.RealwordNum;	
			m_blineNew	   = true;
			m_iNowLine++;
			strline.clear();
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY += nFullW+m_nVertSpac; 		
			OffsetX=m_OffsetX;
			OffsetY=m_OffsetY;
			if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
			{
				if (m_curlineinfo.words.size() > 0)
				{
					pwi->lines.push_back(m_curlineinfo);
					InitLineInfo();
				}
				addstuas=i;
				return 0;
			}			 
		}else
		{
			if (m_blineNew)
				m_blineNew	= false;	
			i+=rtretry;
		}		
	}

	if (strline.size()>0)
	{
		if((iscssEnable)&&(css.textAlign==CENTER)&&(m_OffsetX==m_nMrginLeft)) 
		{ 					
			length1=OffsetX-m_OffsetX;
			length2=m_OutDevW-m_nMrginRight-m_nMrginLeft;
			xxx=(length2-length1)/2+m_nMrginLeft;
			//DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,xxx,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);					
		}else
		{
			//DrawLine(Image,pFont,pEnglishFont,strline,nFullW,nhalfW,UnderlinePos,m_OffsetX,m_OffsetY,m_nWordSpac,contentID,beginoffset,pwi);
		}		
		strline.clear();
		m_OffsetX = OffsetX;				
		m_OffsetY = OffsetY; 
		if (m_OffsetY>(m_OutDevH-m_nMrginBottom-nFullW))
		{
			addstuas=i;
			if (m_curlineinfo.words.size() > 0)
			{
				pwi->lines.push_back(m_curlineinfo);
				InitLineInfo();
			}
			return 0;
		}		
	}	


	
	return 1;
}

int	 SZRenderSimulate::DrawImage2(ImgInfo &pimage)
{
	if (pimage.width<=0||pimage.height<=0)
		return -1;
	m_nImgContentNum++;
	return 1;  
}

int  SZRenderSimulate::DrawImage(ImgInfo &pimage,int x, int y,int Offset,int &addstuas)
{
	addstuas=0;
	int i,j,ii,jj;
	int OffsetX=0;
	int OffsetY=0;
	int ImgH=pimage.height;
	int ImgW=pimage.width;
	if (pimage.width<=0||pimage.height<=0)
		return -1;
	
	
	if (Offset<0/*||Offset>=ImgH*/)
		return -2;
	
	if (x==0&&y==0)
	{
		OffsetX=m_OffsetX;
		OffsetX=0;
		if (ImgW<m_OutDevW)
		{
			OffsetX=max(0,((m_OutDevW-ImgW)/2));
		}else
			OffsetX=0;

		OffsetY=m_OffsetY;
	}else
	{	
		OffsetX=x;
		OffsetY=y;
	}

	Offset=0;
	for (j=Offset;j<ImgH;j++)
	{	
		jj=OffsetY+(j-Offset);//ImgH-1-
		if (jj<(m_OutDevH))//-m_nMrginBottom
		{
			for (i=0;i<ImgW;i++)
			{
				ii=OffsetX+i;
				if (ii>=(m_OutDevW))//-m_nMrginRight
				{
					i+=ImgW;
				}			
			}
		}else
		{
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY  = jj; 
			addstuas   = j;
			m_nImgContentNum++;
			return 0;
		}
	}

	m_OffsetX  = m_nMrginLeft;				
	m_OffsetY  = jj; 
	m_nImgContentNum++;
	return 1;  
}

int SZRenderSimulate::GetOffsetY()
{
	return m_OffsetY;
}

int  SZRender::DrawImage(SZImageBit24  &CanvasImage,SZImageBase*& pimage,int x, int y,int Offset,int imgType, int &addstuas)
{
	addstuas=0;
	int i,j,ii,jj;
	int OffsetX=0;
	int OffsetY=0;
	
	if (pimage==NULL)
		return -1;

	int ImgH=pimage->GetHeight();
	int ImgW=pimage->GetWidth();

	if (imgType==0)
	{	
		if (Offset<0)
			return -2;
		if (x==0&&y==0)
		{
			OffsetX=m_OffsetX;
			OffsetX=0;
			if (ImgW<m_OutDevW)
			{
				OffsetX=max(0,((m_OutDevW-ImgW)/2));
			}else
				OffsetX=0;
			OffsetY=m_OffsetY;
		}else
		{	
			OffsetX=x;
			OffsetY=y;
		} 
	} 
	else if(imgType==1)
	{
		Offset=0;
		OffsetX=max(0,((m_OutDevW-ImgW)/2));
		OffsetY=max(0,((m_OutDevH-ImgH)/2));	 
	}
	
	Offset=0;
	for (j=Offset;j<ImgH;j++)
	{	
		jj=OffsetY+(j-Offset);//ImgH-1-
		if (jj<(m_OutDevH))//-m_nMrginBottom
		{
			for (i=0;i<ImgW;i++)
			{
				ii=OffsetX+i;
				if (ii<(m_OutDevW))//-m_nMrginRight
				{
					CanvasImage.SetDesPixel(ii,m_OutDevH-1-jj,pimage->GetOriPixel(i,ImgH-1-j),255);	
				}else
				{
					i+=ImgW;
				}			
			}
		}else
		{
			m_OffsetX  = m_nMrginLeft;				
			m_OffsetY  = jj; 
			addstuas   = j;
			m_nImgContentNum++;
			return 0;
		}
	}
	m_OffsetX  = m_nMrginLeft;				
	m_OffsetY  = jj; 
	m_nImgContentNum++;
	return 1;  
}

void SZRender::InitLineInfo()
{
	m_curlineinfo.Height = 0;
	m_curlineinfo.Width = 0;
	m_curlineinfo.RealwordNum = 0;
	m_curlineinfo.x = 0;
	m_curlineinfo.y = 0;
	m_curlineinfo.words.clear();
}

int SZRender::SetHighLight(SZImageBit24  &Image, SZFTFont *pFont,SZFTFont *pEFont,vector<wordinfo>& vWordInfo,myRGBQUAD bgColor)
{
	int i;
	int linex(0),liney(0),height(0),width(0);
	int lineMaxY(0);
	int lineIndex(-1);
	bool newLine(false);
	int lineFirstWordoffsetx = 0;
	if (bgColor.rgbReserved == (myBYTE)300)
	{
		goto drawWord;
	}
	for (i=0; i<vWordInfo.size();i++)
	{
		wordinfo wi = vWordInfo[i];
		if (wi.lineIndex > lineIndex)
		{
			if (i!=0)
			{
				height = lineMaxY - liney;
				DrawBackGround(Image,linex,liney,width,height,bgColor);
			}
			linex = wi.x;
			liney = wi.y;
			height = wi.height;
			width = wi.width;
			lineMaxY = wi.y + wi.height;
			lineFirstWordoffsetx = wi.x;
			newLine = true;
			lineIndex = wi.lineIndex;
		}
		else
		{
			
			if (/*wi.height > height*/wi.y + wi.height  > lineMaxY)
			{
				lineMaxY = wi.y +wi.height;
			}
			if (wi.y < liney)
			{
				liney = wi.y;
			}
			width = wi.x - lineFirstWordoffsetx + wi.width;
			
		}
	}
	if (newLine)
	{
		height = lineMaxY - liney;
		DrawBackGround(Image,linex,liney,width,height,bgColor);
	}
	//写字
drawWord:
	int nFullW;
	nFullW =pFont->GetFullWidthAdvanceCH();
	for (i = 0;i < vWordInfo.size();i++)
	{
		wordinfo wi = vWordInfo[i];
		myWORD nowWchar = wi.WordUnicod;
		CharCacheNode* pNode =NULL;
		bool isEngFont;
		if (!isEngChar(nowWchar))
		{
			pNode = pFont->BuildChar(nowWchar);
			isEngFont = false;
		}
		else
		{
			pEFont->BuildCharCache(nowWchar);
			pNode = pEFont->GetCharCacheNode(nowWchar);
			isEngFont = true;
		}	 
		if (!pNode /*|| !pNode->data*/)
		{	 
			continue;			
		}else
		{
			int tempFullW = pFont->GetFullWidthAdvanceCH();
			int y = wi.y -(tempFullW-pNode->y -1);
			RenderChar(Image,pNode,wi.x,y,returnCharLeftRight(nowWchar),isEngFont,tempFullW);
			if (!isEngChar(nowWchar))
			{
				pNode->glyph = NULL;
				delete pNode;
				pNode = NULL;
			}
		}

	}
	return 0;
}

int SZRender::DrawBackGround( SZImageBit24 &Image,int x,int y,int width,int height,myRGBQUAD bgColor)
{
	if (width <= 0 || height <= 0)
	{
		return -1;
	}
	int ImgH=Image.GetHeight();
	int ImgW=Image.GetWidth();
	if (x > ImgW || y > ImgH /*|| x+width > ImgW*/ || y+height > ImgH)
	{
		return -2;
	}
	if (x + width > ImgW)
	{
		width = ImgW - x;
	}
	int i,j;
	for (i=0;i<width;i++)
	{
		for (j=0;j<height;j++)
		{
			Image.SetDesPixel(x + i,ImgH - m_nVertSpac/2 - (y + j),bgColor,255);//此处的3是微调纵坐标
		}
	}
	return 0;
}

void SZRender::CopyOpp( SZRender &opp )
{
	this->m_nTabSize = opp.m_nTabSize; 
	this->m_nVertSpac = opp.m_nVertSpac; 
	this->m_nWordSpac = opp.m_nWordSpac; 
	this->m_nMrginLeft = opp.m_nMrginLeft;  
	this->m_nMrginRight = opp.m_nMrginRight; 
	this->m_nMrginTop = opp.m_nMrginTop;  
	this->m_nMrginBottom = opp.m_nMrginBottom; 
	this->m_nCharContentNum = opp.m_nCharContentNum;
	this->m_nImgContentNum = opp.m_nImgContentNum;
	this->m_nFormat = opp.m_nFormat;	
	this->m_clrForeG = opp.m_clrForeG; 
	this->m_iNowLine = opp.m_iNowLine;
	this->m_blineNew = opp.m_blineNew;
	this->m_nowlineStart = opp.m_nowlineStart;
	this->m_nowlineEnd = opp.m_nowlineEnd;
	this->m_OffsetX = opp.m_OffsetX; 
	this->m_OffsetY = opp.m_OffsetY;	
	this->m_OutDevW = opp.m_OutDevW; 
	this->m_OutDevH = opp.m_OutDevH;
	this->m_fFaceAscender = opp.m_fFaceAscender;
	this->m_curlineinfo = opp.m_curlineinfo;
}
