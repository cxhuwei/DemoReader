// BWImageScalingAlgorithm.h: interface for the BWImageScalingAlgorithm class.
//
//////////////////////////////////////////////////////////////////////
/*
* Copyright (c) 2011,  北京超星研发部 
* All rights reserved.
*
* 文件名称：BWImageScalingAlgorithm.h
* 文件标识：AFX_BWIMAGESCALINGALGORITHM_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_
* 摘 要：   2值位图缩小算法实现
*
* 当前版本：1.0
* 作 者：   邵振     shaozhen1@163.com
* 完成日期：2011年07月26日
*
* 修改记录 : 
* 日 期        版本     修改人              
* 2011/08/02   1.1      邵振                
* 修改内容
* 1.增加了控制输出图像的每像素占用位数bpp
* 2.优化了部分代码结构，使效率提高
* 3.新定义了my64long数据类型和 tagmyBITMAPFILEHEADER数据结构，另外新增接口直接输出bmp文件
* 4.对频繁使用对数据位操作的代码用汇编实现，并用宏定义确定编译环境来判定是否用汇编实现
* 5.为了在移动平台下ARM处理器上改好的提高速度和效率，对使用浮点运算的代码用整型运算来实现，
*   为了提高准确精度，用10^10 倍放大计算,新增接口DoZoom1用来调用之,并保存原有接口DoZoom()
*
*
* 修改记录 : 
* 日 期        版本     修改人              
* 2011/08/12   1.2      邵振                
* 修改内容
* 1.增加了缩小24位真彩图的功能，为了减少判断并利用原来代码，从原有BWImageScalingAlgorithm派生
* Bit24ImageScalingAlgorithm 进行实现
* 2.在测试的过程中发现了整型操作模拟浮点运算时存在bug，会在某些图像缩放时产生竖线现象，通过
* 研究修正了这种情况下的数据运算
*
*
* 修改记录 : 
* 日 期        版本     修改人              
* 2011/08/17   1.3      邵振                
* 修改内容
* 1.增加了缩小8位灰度图的功能  
*
*/
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h> 
#include <memory.h> 
#include <math.h>
#include <malloc.h>
#include "ImageScalingmath.h"
#include "BWImageScalingAlgorithm.h"
 
#define		NULL		  0
#define		BI_RGB        0L
#define		BI_RLE8       1L
#define		BI_RLE4       2L
#define		BI_BITFIELDS  3L
#define		IMG_MAX_MEMORY  256000000
static my64long		g_64longcountSerfurc=1;
static myDWORD		g_lastIndex=0;
static myBYTE		g_lastValve=0;
// FILE *g_pfile= NULL;
// myDWORD g_countdump=0;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BWImageScalingAlgorithm::BWImageScalingAlgorithm()
{
	m_pDib		 =  NULL;
	m_pOriBuffer =  NULL;
	m_pImageBits =  NULL;
	m_bB0W1		 =  false;
	memset(&head,0,sizeof(myBITMAPINFOHEADER));
	m_outImgBitPerPixel=4;
	m_dwOriEffWidth=0;
	dwEffWidth   = 0;  
	m_iOriwidth  = 0;
	m_iOriheight = 0;
	m_iDeswidth  = 0;
	m_iDesheight = 0;
	m_iOptype	 = 0;
	m_ibits		 = 0;
	g_lastIndex  = 0;
	g_lastValve  = 0;  
}

BWImageScalingAlgorithm::~BWImageScalingAlgorithm()
{
	Destroy();
}

bool BWImageScalingAlgorithm::Init(void *inbuf, int Oriwidth, int Oriheight,	int Deswidth, int Desheight,int Optype)
{ 
	return true;
}
bool BWImageScalingAlgorithm::Init(void *inbuf, int Oriwidth, int Oriheight, int bits, bool bB0W1,
								   int Deswidth, int Desheight,int Optype,int outImgBitPerPixel)
{ 	
	float xScale;
	if ((inbuf==NULL)||(bits!=1)||
		Oriwidth<=0||Oriheight<=0 )
	{
		return false;
	}

	m_pOriBuffer = inbuf;
	m_ibits		 = bits;
	m_iOptype	 = Optype;
	m_iOriwidth  = Oriwidth;
	m_iOriheight = Oriheight;
	m_bB0W1		 = bB0W1;
	m_outImgBitPerPixel=outImgBitPerPixel;
	//type =0 既定按照Deswidth  Desheight为输出图运算
	//type =1 按照Deswidth 来输出,Desheight会根据变化比率算出
	//type =2 按照Desheight来输出,Deswidth会根据变化比率算出
	if (m_iOptype<0||m_iOptype>2)
	{
		m_iOptype=1;
	}

	if (0==m_iOptype)
	{
		m_iDeswidth  = Deswidth;
		m_iDesheight = Desheight;
	}else if(1==m_iOptype)
	{
		if (Deswidth<1)	
			return false;
		xScale = (float)Oriwidth  / (float)Deswidth;
		m_iDeswidth  = Deswidth;
		m_iDesheight = (int)((float)Oriheight/xScale);
	}else if(2==m_iOptype)
	{
		if (Desheight<1)	
			return false;
		xScale = (float)Oriheight  / (float)Desheight;
		m_iDesheight = Desheight;
		m_iDeswidth = (int)((float)Oriwidth/xScale);
	}
	
	if ( (m_pOriBuffer==NULL)||(m_ibits!=1)||
		m_iDeswidth<=0||m_iDesheight<=0||
		m_iDeswidth>=m_iOriwidth||
		m_iDesheight>=m_iOriheight)
	{
		return false;
	}
	
 	if (!Create(m_iDeswidth,m_iDesheight,m_outImgBitPerPixel))  
		return false;
	m_dwOriEffWidth=((((m_ibits * m_iOriwidth) + 31) / 32) * 4);
	SetGrayPalette(); 
	g_lastIndex=0;
	g_lastValve=0; 
	return true;
}

void* BWImageScalingAlgorithm::Create(myDWORD dwWidth, myDWORD dwHeight, myBYTE wBpp)
{
	Destroy();
	
	if ((dwWidth == 0) || (dwHeight == 0))
		return NULL;
 
    if		(wBpp <= 1)	wBpp = 1;
    else if (wBpp <= 4)	wBpp = 4;
    else if (wBpp <= 8)	wBpp = 8;
    else				wBpp = 24;
	
 
	if (((dwWidth*dwHeight*wBpp)>>3) > IMG_MAX_MEMORY ||
		((dwWidth*dwHeight*wBpp)/wBpp) != (dwWidth*dwHeight))
	{ 
		return NULL;
	}
	
    switch (wBpp)
	{
	case 1:
		head.biClrUsed = 2;	break;
	case 4:
		head.biClrUsed = 16; break;
	case 8:
		head.biClrUsed = 256; break;
	default:
		head.biClrUsed = 0;
    }	
	
    dwEffWidth = ((((wBpp * dwWidth) + 31) / 32) * 4);
	
    // initialize BITMAPINFOHEADER
	head.biSize   = sizeof(myBITMAPINFOHEADER); 
    head.biWidth  = dwWidth;	
    head.biHeight = dwHeight;		 
    head.biPlanes = 1;				 
    head.biBitCount    = (myWORD)wBpp;	 
    head.biCompression = BI_RGB;    
    head.biSizeImage = dwEffWidth * dwHeight;
	head.biXPelsPerMeter = (long) floor(96 * 10000.0 / 254.0 + 0.5);
	head.biYPelsPerMeter = (long) floor(96 * 10000.0 / 254.0 + 0.5); 
	 
	m_pDib = malloc(GetSize()); 
    if (!m_pDib)
	{	 
		return NULL;
	}
	
	myRGBQUAD* pal=GetPalette();
	if (pal) memset(pal,0,GetPaletteSize());
	
 //   myBITMAPINFOHEADER*  lpbi;
//   	lpbi = (myBITMAPINFOHEADER*)((myBYTE *)m_pDib+sizeof(myBITMAPFILEHEADER));
//       *lpbi = head;
	myBYTE *lpbi = (myBYTE *)m_pDib + sizeof(myBITMAPFILEHEADER);
    memcpy(lpbi, &head, sizeof(myBITMAPINFOHEADER));

	myBITMAPFILEHEADER	hdr;	
	hdr.bfType			= 0x4d42;  
	hdr.bfReserved1		= hdr.bfReserved2 = 0;
	myDWORD bfOffBits	= sizeof(myBITMAPFILEHEADER)+ sizeof(myBITMAPINFOHEADER) + GetPaletteSize();
	hdr.bfOffBitslow	= myWORD(bfOffBits&0xFFFF);
	hdr.bfOffBitshigh	= myWORD((bfOffBits & 0xFFFF0000) >> 16);	
	bfOffBits			= GetSize();
	hdr.bfSizelow		= myWORD(bfOffBits&0xFFFF);
	hdr.bfSizehigh		= myWORD((bfOffBits & 0xFFFF0000) >> 16);
	myBITMAPFILEHEADER*  lpbf;
	lpbf = (myBITMAPFILEHEADER*)(m_pDib);
      *lpbf = hdr;
 
	m_pImageBits=GetBits();	
    return m_pDib;  
}

void BWImageScalingAlgorithm::Destroy()
{
	if (m_pDib!=0)	
	{
		free(m_pDib); 
		m_pDib=0;
	}
}

long BWImageScalingAlgorithm::GetSize()
{
	return head.biSize + head.biSizeImage + GetPaletteSize()+sizeof(myBITMAPFILEHEADER);
}

long BWImageScalingAlgorithm::GetImageSize()
{
	return head.biSizeImage;
}

int	BWImageScalingAlgorithm::GetDstWidth() const
{
	return m_iDeswidth;
}

int	BWImageScalingAlgorithm::GetDstHeight() const
{
	return m_iDesheight;	
}

int	BWImageScalingAlgorithm::GetDstEffWidth() const
{
	return dwEffWidth;
}

void* BWImageScalingAlgorithm::GetDIB() const
{
	return (myBYTE *)m_pDib+sizeof(myBITMAPFILEHEADER);
}

void* BWImageScalingAlgorithm::GetBMP() const
{
	return m_pDib;
}

myBYTE* BWImageScalingAlgorithm::GetBits(myDWORD row)
{ 
	if (m_pDib)
	{
		if (row) {
			if (row<(myDWORD)head.biHeight){
				return ((myBYTE*)m_pDib+sizeof(myBITMAPFILEHEADER)+sizeof(myBITMAPINFOHEADER)+GetPaletteSize()+(dwEffWidth * row));
			} else {return NULL;}
		} 
		else{return ((myBYTE*)m_pDib+sizeof(myBITMAPFILEHEADER)+sizeof(myBITMAPINFOHEADER)+GetPaletteSize());}
	}
	return NULL;
}

myRGBQUAD* BWImageScalingAlgorithm::GetPalette() const
{
	if ((m_pDib)&&(head.biClrUsed))
		return (myRGBQUAD*)((myBYTE*)m_pDib+sizeof(myBITMAPFILEHEADER) + sizeof(myBITMAPINFOHEADER));
	return NULL;
}

myDWORD BWImageScalingAlgorithm::GetPaletteSize()
{
	return (head.biClrUsed * sizeof(myRGBQUAD));
}

void BWImageScalingAlgorithm::SetGrayPalette()
{
	if ((m_pDib==NULL)||(head.biClrUsed==0))
		return;
	myRGBQUAD* pal=GetPalette();
	for (myDWORD ni=0;ni<head.biClrUsed;ni++)
		pal[ni].rgbBlue=pal[ni].rgbGreen = pal[ni].rgbRed = (myBYTE)(ni*(255/(head.biClrUsed-1)));
}

myBYTE BWImageScalingAlgorithm::GetNearestIndex(myBYTE c)
{
	if ((m_pDib==NULL)||(head.biClrUsed==0)) 
		return 0;
	 
	if (g_lastValve==c)
	{
		return (myBYTE)g_lastIndex;
	}
 
	g_lastValve=c;
	myRGBQUAD* pal=GetPalette();
	long k,distance=256; 
	int  j = 0;
	for (myDWORD ni=0;ni<head.biClrUsed;ni++)
	{
		k = abs(c-pal[ni].rgbRed);
		if (k==0){			
			j=ni;
			break;
		}
		if (k<distance){
			distance=k;
			j=ni;
		}
	}
	g_lastIndex=j;
	return (myBYTE)j;
}

bool BWImageScalingAlgorithm::SetDesPixelIndex(long x,long y,myBYTE i)
{
	if ((m_pDib==NULL)||(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight))
		return false;
	
	if (head.biClrUsed)
	{ 
		myBYTE c=GetNearestIndex(i);
		if (head.biBitCount==8){
			m_pImageBits[y*dwEffWidth + x]=c;			
		}
		else 
		{
			myBYTE pos;
			myBYTE* iDst= m_pImageBits + y*dwEffWidth + (x*head.biBitCount >> 3);
			if (head.biBitCount==4)
			{
 				pos = (myBYTE)(4*(1-x%2));
// 				*iDst &= ~(0x0F<<pos);
// 				*iDst |= ((c & 0x0F)<<pos);		
				if (pos==4)
				{
					*iDst &= 0x0F;
					*iDst |=((c & 0x0F)<<4);
				} 
				else
				{
					*iDst &= 0xF0;
					*iDst |=c & 0x0F;
				}
			}
			else if (head.biBitCount==1)
			{
				pos = (myBYTE)(7-x%8);
				*iDst &= ~(0x01<<pos);
				*iDst |= ((c & 0x01)<<pos);
			}
		} 
	}	
	else 
	{
		myBYTE* iDst = m_pImageBits + y*dwEffWidth + x*3;
		*iDst++ =i;
		*iDst++ =i;
		*iDst   =i;
	}
	return true;
}

myBYTE	BWImageScalingAlgorithm::GetOriPixelGray(long x,long y)
{	 
	myBYTE pos=0;
	myBYTE rt=0;
	if(NULL==m_pOriBuffer||m_ibits!=1)
		return 0;
	 
//	if (!(0<=y && y<m_iOriheight && 0<=x && x<m_iOriwidth))
	if (y<0 || y>=m_iOriheight || x<0 || x>=m_iOriwidth)
	{
		x=max(x,(long)0); x=min(x, (long)m_iOriwidth-1);
		y=max(y,(long)0); y=min(y, (long)m_iOriheight-1);
	}
			
	myBYTE iDst=*((myBYTE*) m_pOriBuffer+y*m_dwOriEffWidth + (x*m_ibits >> 3));
// 	if (m_ibits==4)
// 	{
//  		pos = (myBYTE)(4*(1-x%2));
//   		iDst &= (0x0F<<pos);
// 	  	rt= (myBYTE)(iDst >> pos);
// 	} 
// 	else 		if (m_ibits==1)
	{
   		pos = (myBYTE)(7-x%8);
 #if defined (_WIN32) || defined(WIN32)
		__asm
		{
			mov al, iDst;
			mov cl, pos;		 
			shr al, cl;
			and al, 1h;  
			mov rt, al; 
		} 
#else /* Linux */
		iDst &= (0x01<<pos);
  	 	rt= (myBYTE)(iDst >> pos);	
#endif	
	}

// 	if (m_bB0W1)
// 	{
// 		return (rt==1)?255:0;
// 	}else
// 	{
// 		return (rt==1)?0:255;
// 	}

	if ((m_bB0W1&&(rt==1))||
		(!m_bB0W1&&(rt==0)))
	{
		return 255;
	}
	 
	return 0;
}

myBYTE BWImageScalingAlgorithm::GetAreaColorInterpolated1( float const xc, float const yc, float const w, float const h) 
{ 
	int			i,j,count=0;   
	float		countGray=0;  
 
	int xi1=(int)(xc-w/2.0f+0.49999999f);                //low x
	int yi1=(int)(yc-h/2.0f+0.49999999f);                //low y 
	int xi2=(int)(xc+w/2.0f+0.5f);                      //top x
	int yi2=(int)(yc+h/2.0f+0.5f);                      //top y
	
	for (j=yi1;j<yi2;j++)
	{
		for (i=xi1;i<xi2;i++)
		{
			count++;
			countGray+= GetOriPixelGray(i,j);
		}
	}
	if (count>0)
		countGray/=count; 
	
// 	if (countGray>255)
// 		countGray=255;  
	return (myBYTE) countGray; 
}

myBYTE BWImageScalingAlgorithm::GetAreaColorInterpolated11(myDWORD const xc, myDWORD const yc, myDWORD const w, myDWORD const h) 
{	 
	myDWORD		i,j,count =0;   
	myDWORD  	countGray =0;  	
	
	myDWORD xi1=((xc-w/2+50004)/100000);	  //low x
	myDWORD yi1=((yc-h/2+50004)/100000);	  //low y 
	myDWORD xi2=((xc+w/2+50030)/100000);	  //top x
	myDWORD yi2=((yc+h/2+50030)/100000);	  //top y
	
	for (j=yi1;j<yi2;j++)
	{
		for (i=xi1;i<xi2;i++)
		{
			count++;
			countGray+= GetOriPixelGray(i,j);
		}
	}	
	if (count>0)
		countGray/=count; 
	
// 	if (countGray>255)
// 		countGray=255;  
	return (myBYTE) countGray; 
}

myBYTE BWImageScalingAlgorithm::GetAreaColorInterpolated2(float const xc, float const yc, float const w, float const h) 
{
	myBYTE gray=0;      
	float countGray=0;  
  	
  	SS_Rect2 area(xc-w/2.0f, yc-h/2.0f, xc+w/2.0f, yc+h/2.0f);//area
  	int xi1=(int)(area.botLeft.x+0.49999999f);                //low x
  	int yi1=(int)(area.botLeft.y+0.49999999f);                //low y 
	int xi2=(int)(area.topRight.x+0.5f);                      //top x
	int yi2=(int)(area.topRight.y+0.5f);                      //top y 
 
	int x,y;                                                  
	float s=0;                                                //surface of all pixels
 
	SS_Rect2 intBL, intTR;     //bottom left and top right intersection
	intBL=area.CrossSection(SS_Rect2(((float)xi1)-0.5f, ((float)yi1)-0.5f, ((float)xi1)+0.5f, ((float)yi1)+0.5f));
	intTR=area.CrossSection(SS_Rect2(((float)xi2)-0.5f, ((float)yi2)-0.5f, ((float)xi2)+0.5f, ((float)yi2)+0.5f));
	float wBL, wTR, hBL, hTR;
	wBL=intBL.Width();            //width of bottom left pixel-area intersection
	hBL=intBL.Height();           //height of bottom left...
	wTR=intTR.Width();            //width of top right...
	hTR=intTR.Height();           //height of top right...		
	
	countGray+=GetOriPixelGray(xi1,yi1)*wBL*hBL;
	countGray+=GetOriPixelGray(xi2,yi1)*wTR*hBL;
	countGray+=GetOriPixelGray(xi1,yi2)*wBL*hTR;
	countGray+=GetOriPixelGray(xi2,yi2)*wTR*hTR;
	
	//bottom and top row
	for (x=xi1+1; x<xi2; x++) 
	{
		countGray+=GetOriPixelGray(x,yi1)*hBL;
		countGray+=GetOriPixelGray(x,yi2)*hTR;		 
	}
	
	//leftmost and rightmost column
	for (y=yi1+1; y<yi2; y++) 
	{
		countGray+=GetOriPixelGray(xi1,y)*wBL;
		countGray+=GetOriPixelGray(xi2,y)*wTR;			 
	}
	
	for (y=yi1+1; y<yi2; y++) 
	{
		for (x=xi1+1; x<xi2; x++) 
		{ 
			countGray+=GetOriPixelGray(x,y);
		}
	}
 
 	s=area.Surface(); 
	if (s>0)
	{
		countGray/=s; 
		if (countGray>255)
			countGray=255; 
		if (countGray<0)
			countGray=0; 
		gray=(myBYTE) countGray;  
	} 
 
	return gray;
}

myBYTE BWImageScalingAlgorithm::GetAreaColorInterpolated22(int const xc, int const yc, int const w, int const h)
{
	myDWORD  				   countGray=0; 
	my64long		 	 countGray100000=0;
	my64long		countGray10000000000=0; 
	
	int xi1a=(xc-w/2);
	int yi1a=(yc-h/2);
	int xi2a=(xc+w/2);
	int yi2a=(yc+h/2);

	int xi1b=(xi1a+50004);
	int yi1b=(yi1a+50004);
	int xi2b=(xi2a+50030);
	int yi2b=(yi2a+50030);

	int xi1=xi1b/100000;	  //low x
	int yi1=yi1b/100000;	  //low y 
	int xi2=xi2b/100000;	  //top x
	int yi2=yi2b/100000;	  //top y

 	xi1b=xi1*100000;
	yi1b=yi1*100000;
	xi2b=xi2*100000;
	yi2b=yi2*100000;
 
	//bottom left and top right intersection
	int intBLbotLeftx=max(xi1a, (xi1b-50000));
	int intBLbotLefty=max(yi1a, (yi1b-50000));
	int intBLtopRightx=min(xi2a, (xi1b+50000));
	int intBLtopRighty=min(yi2a, (yi1b+50000));
	if (intBLbotLeftx>intBLtopRightx||intBLbotLefty>intBLtopRighty) 
	{
		intBLbotLeftx=intBLbotLefty=intBLtopRightx=intBLtopRighty=0;
	}

	int intTRbotLeftx=max(xi1a, (xi2b-50000));
	int intTRbotLefty=max(yi1a, (yi2b-50000));
	int intTRtopRightx=min(xi2a, (xi2b+50000));
	int intTRtopRighty=min(yi2a, (yi2b+50000));	
	if (intTRbotLeftx>intTRtopRightx||intTRbotLefty>intTRtopRighty) 
	{
		intTRbotLeftx=intTRtopRightx=intTRbotLefty=intTRtopRighty=0;
	}

	int x, y, wBL, wTR, hBL, hTR;
	wBL=intBLtopRightx-intBLbotLeftx;            //width of bottom left pixel-area intersection
	hBL=intBLtopRighty-intBLbotLefty;            //height of bottom left...
	wTR=intTRtopRightx-intTRbotLeftx;            //width of top right...
	hTR=intTRtopRighty-intTRbotLefty;            //height of top right...

	countGray10000000000+=wBL*GetOriPixelGray(xi1,yi1)/100000*hBL;
	countGray10000000000+=wTR*GetOriPixelGray(xi2,yi1)/100000*hBL;
	countGray10000000000+=wBL*GetOriPixelGray(xi1,yi2)/100000*hTR;
	countGray10000000000+=wTR*GetOriPixelGray(xi2,yi2)/100000*hTR; 	

	if (wBL==0||hBL==0||wTR==0||hTR==0)
	{
		int count =0;
		for (y=yi1;y<yi2;y++)
		{
			for (x=xi1;x<xi2;x++)
			{
				count++;
				countGray+=GetOriPixelGray(x,y); 
			}
		}	
		
		if (count>0)
		{
			countGray/=count;  
		}else
		{ 
			countGray= GetOriPixelGray((xi1+xi2)/2,(yi1+yi2)/2);
		}
		return (myBYTE) countGray;  
	}

	//leftmost and rightmost column
	for (y=yi1+1; y<yi2; y++) 
	{
		countGray100000+=GetOriPixelGray(xi1,y)*wBL;
		countGray100000+=GetOriPixelGray(xi2,y)*wTR;			 
	}

	//bottom and top row
	for (x=xi1+1; x<xi2; x++) 
	{
		countGray100000+=GetOriPixelGray(x,yi1)*hBL;
		countGray100000+=GetOriPixelGray(x,yi2)*hTR;		 
	}
 
	for (y=yi1+1; y<yi2; y++) 
	{
		for (x=xi1+1; x<xi2; x++) 
		{ 
			countGray+=GetOriPixelGray(x,y);
		}
	}

  	countGray=(myDWORD)(((countGray+countGray100000/100000+countGray10000000000/100000)*10000000000)/g_64longcountSerfurc);
 
// 	if (countGray>255)
// 		countGray=255; 	
	return (myBYTE) countGray; 
}

bool BWImageScalingAlgorithm::DoZoom1(bool quick)
{ 	
	long  dX,dY;      
	if (m_iOriwidth<=m_iDeswidth ||m_iOriheight<=m_iDesheight) 
	{ 
		return false;
	} 
 
	int		 xScale, yScale,sX,sY;
	xScale = m_iOriwidth*100000/m_iDeswidth;    
	yScale = m_iOriheight*100000/m_iDesheight;	
	g_64longcountSerfurc=(my64long)xScale*(my64long)yScale;
	for(dY=0; dY<m_iDesheight; dY++)
	{
		sY = ((2*dY+1) *yScale-100000)/2;
		for(dX=0; dX<m_iDeswidth; dX++)
		{
			sX =((2*dX+1) *xScale-100000)/2;
			if (quick)
			{
				SetDesPixelIndex(dX,dY,GetAreaColorInterpolated11(sX, sY, xScale, yScale));
			} 
			else
			{
				SetDesPixelIndex(dX,dY,GetAreaColorInterpolated22(sX, sY, xScale, yScale));
			}			
		}
	}
 
	return true;
}

bool BWImageScalingAlgorithm::DoZoom(bool quick)
{	 
	float xScale, yScale;
	xScale = (float)m_iOriwidth / (float)m_iDeswidth;    
	yScale = (float)m_iOriheight / (float)m_iDesheight;
	if (!(xScale>1 && yScale>1)) 
	{ 
		return false;
	} 	
 
	float sX,sY;         
	long  dX,dY;         
	for(dY=0; dY<m_iDesheight; dY++)
	{		
		sY = (dY+0.5f) * yScale - 0.5f;
		for(dX=0; dX<m_iDeswidth; dX++)
		{
			sX = (dX+0.5f) * xScale - 0.5f;
			if (quick)
			{
				SetDesPixelIndex(dX,dY,GetAreaColorInterpolated1(sX, sY, xScale, yScale));
			} 
			else
			{
				SetDesPixelIndex(dX,dY,GetAreaColorInterpolated2(sX, sY, xScale, yScale));
			}			
		}
	}
 
	return true;
}

bool Bit24ImageScalingAlgorithm::Init(void *inbuf, int Oriwidth, int Oriheight,	int Deswidth, int Desheight,int Optype)
{ 
	float xScale;
	
	m_ibits				= 24;
	m_outImgBitPerPixel = 24;
	if ((inbuf==NULL)||Oriwidth<=0||Oriheight<=0 )
	{
		return false;
	}

	m_pOriBuffer = inbuf;	
	m_iOriwidth  = Oriwidth;
	m_iOriheight = Oriheight;
	m_iOptype	 = Optype;
	if (m_iOptype<0||m_iOptype>2)
	{
		m_iOptype=1;
	}

	//type =0 既定按照Deswidth  Desheight为输出图运算
	//type =1 按照Deswidth 来输出,Desheight会根据变化比率算出
	//type =2 按照Desheight来输出,Deswidth会根据变化比率算出
	if (0==m_iOptype)
	{
		m_iDeswidth  = Deswidth;
		m_iDesheight = Desheight;
	}else if(1==m_iOptype)
	{
		if (Deswidth<1)	
			return false;
		xScale = (float)Oriwidth  / (float)Deswidth;
		m_iDeswidth  = Deswidth;
		m_iDesheight = (int)((float)Oriheight/xScale);
	}else if(2==m_iOptype)
	{
		if (Desheight<1)	
			return false;
		xScale = (float)Oriheight  / (float)Desheight;
		m_iDesheight = Desheight;
		m_iDeswidth = (int)((float)Oriwidth/xScale);
	}
	
	if ( m_iDeswidth<=0||m_iDesheight<=0||m_iDeswidth>=m_iOriwidth||m_iDesheight>=m_iOriheight)
	{
		return false;
	}
	
 	if (!Create(m_iDeswidth,m_iDesheight,m_outImgBitPerPixel))  
		return false;
	m_dwOriEffWidth=((((m_ibits * m_iOriwidth) + 31) / 32) * 4);
	SetGrayPalette(); 
	g_lastIndex=0;
	g_lastValve=0; 
	return true;
}

void Bit24ImageScalingAlgorithm::AddAveragingCont(myRGBQUAD const &color, float const surf,
												  float &rr, float &gg, float &bb)
{
	rr+=color.rgbRed*surf;
	gg+=color.rgbGreen*surf;
	bb+=color.rgbBlue*surf;
}

bool Bit24ImageScalingAlgorithm::SetDesPixel(long x,long y,myRGBQUAD i)
{
	if ((m_pDib==NULL)||(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight))
		return false;
	
	if (head.biClrUsed)
	{ 
		return false;
	}	
	else 
	{
		myBYTE* iDst = m_pImageBits + y*dwEffWidth + x*3;
		*iDst++ =i.rgbBlue;
		*iDst++ =i.rgbGreen;
		*iDst   =i.rgbRed;
	}
	return true;
}

myRGBQUAD	Bit24ImageScalingAlgorithm::GetOriPixel(long x,long y)
{	
	myRGBQUAD rgb={0,0,0,0};
	if(NULL==m_pOriBuffer||m_ibits!=24)
		return rgb;
	
	if (y<0 || y>=m_iOriheight || x<0 || x>=m_iOriwidth)
	{
		x=max(x,(long)0); x=min(x, (long)m_iOriwidth-1);
		y=max(y,(long)0); y=min(y, (long)m_iOriheight-1);
	}
	
	myBYTE *iDst=(myBYTE*)m_pOriBuffer+y*m_dwOriEffWidth + x*3; 
	rgb.rgbBlue = *iDst++;
	rgb.rgbGreen= *iDst++;
	rgb.rgbRed  = *iDst;
	return rgb;
}

myRGBQUAD Bit24ImageScalingAlgorithm::GetAreaColorInterpolated1( float const xc, float const yc, float const w, float const h) 
{ 
	int			i,j,count=0;
   	myDWORD		rr,gg,bb;
	myRGBQUAD   color;  
	
	int xi1=(int)(xc-w/2.0f+0.49999999f);               //low x
	int yi1=(int)(yc-h/2.0f+0.49999999f);               //low y 
	int xi2=(int)(xc+w/2.0f+0.5f);                      //top x
	int yi2=(int)(yc+h/2.0f+0.5f);                      //top y

	rr=gg=bb=0;
	for (j=yi1;j<yi2;j++)
	{
		for (i=xi1;i<xi2;i++)
		{
  			count++;
			color= GetOriPixel(i,j);
			rr+=color.rgbRed;
			gg+=color.rgbGreen;
			bb+=color.rgbBlue; 		
		}
	}

 	if (count>0)
	{
		rr/=count; 
		gg/=count; 
		bb/=count; 
	}
 
//	if (rr>255) rr=255; if (rr<0) rr=0; 
	color.rgbRed=(myBYTE) rr;
//	if (gg>255) gg=255; if (gg<0) gg=0; 
	color.rgbGreen=(myBYTE) gg;
//	if (bb>255) bb=255; if (bb<0) bb=0; 
	color.rgbBlue=(myBYTE) bb;
	
	return color; 
}

myRGBQUAD Bit24ImageScalingAlgorithm::GetAreaColorInterpolated2(float const xc, float const yc, float const w, float const h) 
{
	int			x,y;   
	myRGBQUAD   color;    
	float       rr,gg,bb,s=0;    
  	 
  	SS_Rect2 area(xc-w/2.0f, yc-h/2.0f, xc+w/2.0f, yc+h/2.0f);//area
  	int xi1=(int)(area.botLeft.x+0.49999999f);                //low x
  	int yi1=(int)(area.botLeft.y+0.49999999f);                //low y 
	int xi2=(int)(area.topRight.x+0.5f);                      //top x
	int yi2=(int)(area.topRight.y+0.5f);                      //top y 

	SS_Rect2 intBL, intTR;     //bottom left and top right intersection
	intBL=area.CrossSection(SS_Rect2(((float)xi1)-0.5f, ((float)yi1)-0.5f, ((float)xi1)+0.5f, ((float)yi1)+0.5f));
	intTR=area.CrossSection(SS_Rect2(((float)xi2)-0.5f, ((float)yi2)-0.5f, ((float)xi2)+0.5f, ((float)yi2)+0.5f));
	float wBL, wTR, hBL, hTR;
	wBL=intBL.Width();            //width of bottom left pixel-area intersection
	hBL=intBL.Height();           //height of bottom left...
	wTR=intTR.Width();            //width of top right...
	hTR=intTR.Height();           //height of top right...
	
	rr=gg=bb=0;
	AddAveragingCont(GetOriPixel(xi1,yi1), wBL*hBL, rr, gg, bb);
	AddAveragingCont(GetOriPixel(xi2,yi1), wTR*hBL, rr, gg, bb);
	AddAveragingCont(GetOriPixel(xi1,yi2), wBL*hTR, rr, gg, bb);
	AddAveragingCont(GetOriPixel(xi2,yi2), wTR*hTR, rr, gg, bb);
 
	//bottom and top row
	for (x=xi1+1; x<xi2; x++) 
	{ 
		AddAveragingCont(GetOriPixel(x,yi1), hBL, rr, gg, bb);
		AddAveragingCont(GetOriPixel(x,yi2), hTR, rr, gg, bb);
	}
	
	//leftmost and rightmost column
	for (y=yi1+1; y<yi2; y++) 
	{ 
		AddAveragingCont(GetOriPixel(xi1,y), wBL, rr, gg, bb);
		AddAveragingCont(GetOriPixel(xi2,y), wTR, rr, gg, bb);
	}
	
	for (y=yi1+1; y<yi2; y++) 
	{
		for (x=xi1+1; x<xi2; x++) 
		{ 
			color= GetOriPixel(x,y);
			rr+=color.rgbRed;
			gg+=color.rgbGreen;
			bb+=color.rgbBlue; 	
		}
	}
 
  	s=area.Surface(); 
	if (s>0)
	{
		rr/=s; 
		gg/=s;
		bb/=s; 	
	}

	if (rr>255) rr=255; if (rr<0) rr=0; color.rgbRed=(myBYTE) rr;
	if (gg>255) gg=255; if (gg<0) gg=0; color.rgbGreen=(myBYTE) gg;
	if (bb>255) bb=255; if (bb<0) bb=0; color.rgbBlue=(myBYTE) bb;
	return color; 
}


bool Bit24ImageScalingAlgorithm::DoZoom(bool quick)
{	 
	float xScale, yScale;
	xScale = (float)m_iOriwidth / (float)m_iDeswidth;    
	yScale = (float)m_iOriheight / (float)m_iDesheight;
	if (!(xScale>1 && yScale>1)) 
	{ 
		return false;
	} 	
 
	float sX,sY;         
	long  dX,dY;         
	for(dY=0; dY<m_iDesheight; dY++)
	{		
		sY = (dY+0.5f) * yScale - 0.5f;
		for(dX=0; dX<m_iDeswidth; dX++)
		{
			sX = (dX+0.5f) * xScale - 0.5f;
			if (quick)
			{
				SetDesPixel(dX,dY,GetAreaColorInterpolated1(sX, sY, xScale, yScale));
			} 
			else
			{
				SetDesPixel(dX,dY,GetAreaColorInterpolated2(sX, sY, xScale, yScale));
			}			
		}
	} 
	return true;
}

bool Bit24ImageScalingAlgorithm::DoZoom1(bool quick)
{ 	
	long  dX,dY;      
	if (m_iOriwidth<=m_iDeswidth ||m_iOriheight<=m_iDesheight) 
	{ 
		return false;
	} 

	int		 xScale, yScale,sX,sY;
	xScale = m_iOriwidth*100000/m_iDeswidth;    
	yScale = m_iOriheight*100000/m_iDesheight;	
	g_64longcountSerfurc=(my64long)xScale*(my64long)yScale;
	
// 	g_pfile=fopen("e:\\outdump.txt","wb");
// 	g_countdump=0;
	for(dY=0; dY<m_iDesheight; dY++)
	{
		sY = ((2*dY+1) *yScale-100000)/2;
		for(dX=0; dX<m_iDeswidth; dX++)
		{
			sX =((2*dX+1) *xScale-100000)/2;
			if (quick)
			{
				SetDesPixel(dX,dY,GetAreaColorInterpolated11(sX, sY, xScale, yScale));
			} 
			else
			{
				SetDesPixel(dX,dY,GetAreaColorInterpolated22(sX, sY, xScale, yScale));
			}			
		}
	}	
// 	if (g_pfile)
// 	{
// 		fclose(g_pfile);
// 		g_pfile=NULL;
// 	}
	return true;
}


myRGBQUAD Bit24ImageScalingAlgorithm::GetAreaColorInterpolated11(myDWORD const xc, myDWORD const yc,
																 myDWORD const w, myDWORD const h) 
{	 
	myDWORD		i,j,rr,gg,bb,count =0;   
	myRGBQUAD   color;   	
	
	myDWORD xi1=((xc-w/2+50004)/100000);	  //low x
	myDWORD yi1=((yc-h/2+50004)/100000);	  //low y 
	myDWORD xi2=((xc+w/2+50030)/100000);	  //top x
	myDWORD yi2=((yc+h/2+50030)/100000);	  //top y
	
	rr=gg=bb=0;
	for (j=yi1;j<yi2;j++)
	{
		for (i=xi1;i<xi2;i++)
		{
			count++;
			color= GetOriPixel(i,j);
			rr+=color.rgbRed;
			gg+=color.rgbGreen;
			bb+=color.rgbBlue; 	
		}
	}	
	if (count>0)
	{
		rr/=count; 
		gg/=count; 
		bb/=count; 
	}
	
//	if (rr>255) rr=255; if (rr<0) rr=0; 
	color.rgbRed=(myBYTE) rr;
//	if (gg>255) gg=255; if (gg<0) gg=0; 
	color.rgbGreen=(myBYTE) gg;
//	if (bb>255) bb=255; if (bb<0) bb=0; 
	color.rgbBlue=(myBYTE) bb;
	
	return color; 
}

myRGBQUAD Bit24ImageScalingAlgorithm::GetAreaColorInterpolated22(int const xc, int const yc, int const w, int const h)
{
	myRGBQUAD	 color;   
	int			 x, y, wBL, wTR, hBL, hTR;
	my64long	 rr,gg,bb,rr100000,gg100000,bb100000,rr10000000000,gg10000000000,bb10000000000;
	rr=gg=bb=rr100000=gg100000=bb100000=rr10000000000=gg10000000000=bb10000000000=0;
	
	int xi1a=(xc-w/2);
	int yi1a=(yc-h/2);
	int xi2a=(xc+w/2);
	int yi2a=(yc+h/2);
	
	int xi1b=(xi1a+50004);
	int yi1b=(yi1a+50004);
	int xi2b=(xi2a+50030);
	int yi2b=(yi2a+50030);
	
	int xi1=xi1b/100000;	  //low x
	int yi1=yi1b/100000;	  //low y 
	int xi2=xi2b/100000;	  //top x
	int yi2=yi2b/100000;	  //top y
	
	xi1b=xi1*100000;
	yi1b=yi1*100000;
	xi2b=xi2*100000;
	yi2b=yi2*100000;
	
	//bottom left and top right intersection
	int intBLbotLeftx=max(xi1a, (xi1b-50000));
	int intBLbotLefty=max(yi1a, (yi1b-50000));
	int intBLtopRightx=min(xi2a, (xi1b+50000));
	int intBLtopRighty=min(yi2a, (yi1b+50000));
	if (intBLbotLeftx>intBLtopRightx||intBLbotLefty>intBLtopRighty) 
	{
		intBLbotLeftx=intBLbotLefty=intBLtopRightx=intBLtopRighty=0;
	}
	
	int intTRbotLeftx=max(xi1a, (xi2b-50000));
	int intTRbotLefty=max(yi1a, (yi2b-50000));
	int intTRtopRightx=min(xi2a, (xi2b+50000));
	int intTRtopRighty=min(yi2a, (yi2b+50000));	
 
	if (intTRbotLeftx>intTRtopRightx||intTRbotLefty>intTRtopRighty) 
	{
		intTRbotLeftx=intTRtopRightx=intTRbotLefty=intTRtopRighty=0;
	}
 
	wBL=intBLtopRightx-intBLbotLeftx;            //width of bottom left pixel-area intersection
	hBL=intBLtopRighty-intBLbotLefty;            //height of bottom left...
	wTR=intTRtopRightx-intTRbotLeftx;            //width of top right...
	hTR=intTRtopRighty-intTRbotLefty;            //height of top right...
	if (wBL==0||hBL==0||wTR==0||hTR==0)
	{
		int count =0;
		for (y=yi1;y<yi2;y++)
		{
			for (x=xi1;x<xi2;x++)
			{
				count++;
				color= GetOriPixel(x,y);
				rr+=color.rgbRed;
				gg+=color.rgbGreen;
				bb+=color.rgbBlue; 	
			}
		}	
		
		if (count>0)
		{
			rr/=count; 
			gg/=count; 
			bb/=count; 
			color.rgbRed=(myBYTE) rr;	 
			color.rgbGreen=(myBYTE) gg;	
			color.rgbBlue=(myBYTE) bb;
		}else
		{ 
			color= GetOriPixel((xi1+xi2)/2,(yi1+yi2)/2);
		}
// 		if (g_pfile&&g_countdump<20)
// 		{
// 			fprintf(g_pfile,"xc=%d,yc=%d,w=%d,h=%d,xi1a=%d,yi1a=%d,xi2a=%d,yi2a=%d,\
// 				xi1b=%d,yi1b=%d,xi2b=%d,yi2b=%d,xi1=%d,yi1=%d,xi2=%d,yi2=%d,\
// 				intBLbotLeftx=%d,intBLbotLefty=%d,intBLtopRightx=%d,intBLtopRighty=%d,\
// 				intTRbotLeftx=%d,intTRtopRightx=%d,intTRbotLefty=%d,intTRtopRighty=%d,\
// 				wBL=%d,wTR=%d,hBL=%d,hTR=%d\r\n",
// 				xc,yc,w,h,xi1a,yi1a,xi2a,yi2a,xi1b,yi1b,xi2b,yi2b,xi1,yi1,xi2,yi2,
// 				intBLbotLeftx,intBLbotLefty,intBLtopRightx,intBLtopRighty,
// 				intTRbotLeftx,intTRtopRightx,intTRbotLefty,intTRtopRighty,
// 				wBL, wTR, hBL, hTR);	
// 			g_countdump++;
// 		}
// 		color.rgbBlue=0;
// 		color.rgbGreen=0;
// 		color.rgbRed=255;

  		return color; 
	}

	color= GetOriPixel(xi1,yi1);
	rr10000000000+=wBL*color.rgbRed/100000*hBL;
	gg10000000000+=wBL*color.rgbGreen/100000*hBL;
	bb10000000000+=wBL*color.rgbBlue/100000*hBL; 	
	
	color= GetOriPixel(xi2,yi1);
	rr10000000000+=wTR*color.rgbRed/100000*hBL;
	gg10000000000+=wTR*color.rgbGreen/100000*hBL;
	bb10000000000+=wTR*color.rgbBlue/100000*hBL; 
	
	color= GetOriPixel(xi1,yi2);
	rr10000000000+=wBL*color.rgbRed/100000*hTR;
	gg10000000000+=wBL*color.rgbGreen/100000*hTR;
	bb10000000000+=wBL*color.rgbBlue/100000*hTR; 
	
	color= GetOriPixel(xi2,yi2);
	rr10000000000+=wTR*color.rgbRed/100000*hTR;
	gg10000000000+=wTR*color.rgbGreen/100000*hTR;
	bb10000000000+=wTR*color.rgbBlue/100000*hTR; 
	
	//bottom and top row
	for (x=xi1+1; x<xi2; x++) 
	{
		color= GetOriPixel(x,yi1);
		rr100000+=color.rgbRed*hBL;
		gg100000+=color.rgbGreen*hBL;
		bb100000+=color.rgbBlue*hBL; 
		
		color= GetOriPixel(x,yi2);
		rr100000+=color.rgbRed*hTR;
		gg100000+=color.rgbGreen*hTR;
		bb100000+=color.rgbBlue*hTR; 
	}
	
	//leftmost and rightmost column
	for (y=yi1+1; y<yi2; y++) 
	{ 
		color= GetOriPixel(xi1,y);
		rr100000+=color.rgbRed*wBL;
		gg100000+=color.rgbGreen*wBL;
		bb100000+=color.rgbBlue*wBL; 
		
		color= GetOriPixel(xi2,y);
		rr100000+=color.rgbRed*wTR;
		gg100000+=color.rgbGreen*wTR;
		bb100000+=color.rgbBlue*wTR; 
	}
 
	for (y=yi1+1; y<yi2; y++) 
	{
		for (x=xi1+1; x<xi2; x++) 
		{  
			color= GetOriPixel(x,y);
			rr+=color.rgbRed;
			gg+=color.rgbGreen;
			bb+=color.rgbBlue; 	
		}
	} 

	color.rgbRed=(myBYTE)(((rr+(rr100000+rr10000000000)/100000)*10000000000)/g_64longcountSerfurc);  
	color.rgbGreen=(myBYTE)(((gg+(gg100000+gg10000000000)/100000)*10000000000)/g_64longcountSerfurc);  
	color.rgbBlue=(myBYTE)(((bb+(bb100000+bb10000000000)/100000)*10000000000)/g_64longcountSerfurc);  
	return color; 	 
}

bool Bit8ImageScalingAlgorithm::Init(void *inbuf, int Oriwidth, int Oriheight,	int Deswidth, int Desheight,int Optype)
{ 
	float xScale;	
	m_ibits				= 8;
	m_outImgBitPerPixel = 8;
	if ((inbuf==NULL)||Oriwidth<=0||Oriheight<=0 )
	{
		return false;
	}
	
	m_pOriBuffer = inbuf;	
	m_iOriwidth  = Oriwidth;
	m_iOriheight = Oriheight;
	m_iOptype	 = Optype;
	if (m_iOptype<0||m_iOptype>2)
	{
		m_iOptype=1;
	}
	
	//type =0 既定按照Deswidth  Desheight为输出图运算
	//type =1 按照Deswidth 来输出,Desheight会根据变化比率算出
	//type =2 按照Desheight来输出,Deswidth会根据变化比率算出
	if (0==m_iOptype)
	{
		m_iDeswidth  = Deswidth;
		m_iDesheight = Desheight;
	}else if(1==m_iOptype)
	{
		if (Deswidth<1)	
			return false;
		xScale = (float)Oriwidth  / (float)Deswidth;
		m_iDeswidth  = Deswidth;
		m_iDesheight = (int)((float)Oriheight/xScale);
	}else if(2==m_iOptype)
	{
		if (Desheight<1)	
			return false;
		xScale = (float)Oriheight  / (float)Desheight;
		m_iDesheight = Desheight;
		m_iDeswidth = (int)((float)Oriwidth/xScale);
	}
	
	if ( m_iDeswidth<=0||m_iDesheight<=0||m_iDeswidth>=m_iOriwidth||m_iDesheight>=m_iOriheight)
	{
		return false;
	}
	
	if (!Create(m_iDeswidth,m_iDesheight,m_outImgBitPerPixel))  
		return false;
	m_dwOriEffWidth=((((m_ibits * m_iOriwidth) + 31) / 32) * 4);
	SetGrayPalette(); 
	g_lastIndex=0;
	g_lastValve=0; 
	return true;
}

myBYTE	Bit8ImageScalingAlgorithm::GetOriPixelGray(long x,long y)
{	 
	myBYTE rt=0;
	if(NULL==m_pOriBuffer||m_ibits!=8)
		return 0;
	
	if (y<0 || y>=m_iOriheight || x<0 || x>=m_iOriwidth)
	{
		x=max(x,(long)0); x=min(x, (long)m_iOriwidth-1);
		y=max(y,(long)0); y=min(y, (long)m_iOriheight-1);
	}
 
	myBYTE *iDst=(myBYTE*)m_pOriBuffer+y*m_dwOriEffWidth + x; 
	rt = *iDst; 
	return rt;
}
 
