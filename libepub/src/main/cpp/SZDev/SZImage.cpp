/*
* Copyright (c) 2011,  北京超星研发部 
* All rights reserved.
*
* 文件名称：SZImage.cpp
* 文件标识：AFX_BWIMAGESCALINGALGORITHM_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_
* 摘 要：   简单位图处理类实现
*
* 当前版本：1.0
* 作 者：   邵振     shaozhen1@163.com
* 完成日期：2011年08月31日
*
*
*/
#include "StdAfx.h"
#include <stdlib.h>
#include <stdio.h> 
#include <memory.h> 
#include <math.h>
//#include <malloc.h> 
#include "SZImage.h"

#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef   NULL
#define		NULL			0
#endif


#define		BI_RGB			0L
#define		BI_RLE8			1L
#define		BI_RLE4			2L
#define		BI_BITFIELDS	3L
#define		IMG_MAX_MEMORY  256000000
myRGBQUAD  SZImageBase::MakeRGBQuad(myBYTE r,myBYTE g,myBYTE b)
{
	myRGBQUAD rt={0,0,0,0};
	rt.rgbBlue=b;
	rt.rgbGreen=g;
	rt.rgbRed=r;
	return rt;
}

myRGBQUAD SZImageBase::MakeRGBQuad( myDWORD color )
{
	myRGBQUAD rt = {0,0,0,0};
	rt.rgbBlue = color&0x000000ff;
	rt.rgbGreen = (color>>8)&0xff;
	rt.rgbRed = (color>>16)&0xff;
	return rt;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SZImageBase::SZImageBase()
{ 
	m_pDib		 =  NULL; 
	m_pImageBits =  NULL;
	memset(&head,0,sizeof(myBITMAPINFOHEADER)); 
	m_dwEffWidth      = 0;  
	m_iWidth          = 0;
	m_iHeight         = 0;  

	short test = 1;
	m_bLittleEndianHost = (*((char *) &test) == 1);
} 

SZImageBase::~SZImageBase()
{
	Destroy();
}

void SZImageBase::Destroy()
{
	if (m_pDib!=0)	
	{
		free(m_pDib); 
		m_pDib=0;
	}
}

int	SZImageBase::GetBitCount()const
{
	if (m_pDib!=0)	
	{
		 return (int)(head.biBitCount);
	}
	return 0;
}

void* SZImageBase::Create(myDWORD dwWidth, myDWORD dwHeight, myBYTE wBpp)
{
	Destroy();
	
	if ((dwWidth == 0) || (dwHeight == 0))
		return NULL;
 
    if		(wBpp <= 1)	wBpp = 1;
    else if (wBpp <= 4)	wBpp = 4;
    else if (wBpp <= 8)	wBpp = 8;
    /*else				wBpp = 24;*/
	else if (wBpp <= 24) wBpp = 24;
	else				wBpp = 32;

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
	
    m_dwEffWidth = ((((wBpp * dwWidth) + 31) / 32) * 4);
	
 	head.biSize   = sizeof(myBITMAPINFOHEADER); 
    head.biWidth  = dwWidth;	
    head.biHeight = dwHeight;		 
    head.biPlanes = 1;				 
    head.biBitCount    = (myWORD)wBpp;	 
    head.biCompression = BI_RGB;    
    head.biSizeImage = m_dwEffWidth * dwHeight;
	head.biXPelsPerMeter = (long) floor(96 * 10000.0 / 254.0 + 0.5);
	head.biYPelsPerMeter = (long) floor(96 * 10000.0 / 254.0 + 0.5); 
	 
	m_pDib = malloc(GetSize()); 
    if (!m_pDib)
	{	 
		return NULL;
	}

	memset(m_pDib,0,GetSize());
	myRGBQUAD* pal=GetPalette();
	if (pal)
		memset(pal,0,GetPaletteSize());
	
//    myBITMAPINFOHEADER*  lpbi;
//  	lpbi = (myBITMAPINFOHEADER*)((myBYTE *)m_pDib+sizeof(myBITMAPFILEHEADER));
//      *lpbi = head;
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

long SZImageBase::GetSize() const
{
	return sizeof(myBITMAPFILEHEADER) + head.biSize + head.biSizeImage + GetPaletteSize();
}
 
void* SZImageBase::GetDIB() const
{
	return (myBYTE *)m_pDib+sizeof(myBITMAPFILEHEADER);
}

void* SZImageBase::GetBMP() const
{
	return m_pDib;
}

myBYTE* SZImageBase::GetBits(myDWORD row) const
{ 
	if (m_pDib)
	{	
		if (row) 
		{
			if (row<(myDWORD)head.biHeight)
			{
				return ((myBYTE*)m_pDib+sizeof(myBITMAPFILEHEADER)+sizeof(myBITMAPINFOHEADER)+GetPaletteSize()+(m_dwEffWidth * row));
			} 
			else return NULL;
		} 
		else return ((myBYTE*)m_pDib+sizeof(myBITMAPFILEHEADER)+sizeof(myBITMAPINFOHEADER)+GetPaletteSize());
	}
	return NULL;
}

void SZImageBase::SetBits(myBYTE* pdata,myDWORD offset,myDWORD size)
{
	long realsize=size;
	if (m_pImageBits&&(offset<head.biSizeImage))
	{ 		
		if((offset+realsize)>head.biSizeImage)
		{
		  realsize=head.biSizeImage-offset;
		}  
		memcpy(m_pImageBits+offset,pdata,realsize);
	} 
}

bool SZImageBase::SetGrayPalette(myDWORD index,myRGBQUAD color)
{
	if ((m_pDib==NULL)||(head.biClrUsed==0))
		return false;
	myRGBQUAD* pal=GetPalette(index);
	if (pal==NULL)
	{
		return false;
	} 
	else
	{
		pal->rgbBlue  =color.rgbBlue;
		pal->rgbGreen =color.rgbGreen;
		pal->rgbRed   =color.rgbRed;	 
		return true;
	} 
}

void SZImageBase::InitGrayPalette()
{
	if ((m_pDib==NULL)||(head.biClrUsed==0))
		return;
	myRGBQUAD* pal=GetPalette();
	if (pal)
	{
		for (myDWORD ni=0;ni<head.biClrUsed;ni++)
			pal[ni].rgbBlue=pal[ni].rgbGreen = pal[ni].rgbRed = (myBYTE)(ni*(255/(head.biClrUsed-1)));
	}
} 

myRGBQUAD* SZImageBase::GetPalette(myDWORD index) const
{
	if ((m_pDib)&&(head.biClrUsed))
	{
		if (index) 
		{
			if (index<head.biClrUsed)
			{
				return ((myRGBQUAD*)((myBYTE*)m_pDib+sizeof(myBITMAPFILEHEADER) + sizeof(myBITMAPINFOHEADER)))+index;
			} 
			else return NULL;
		} 
		else return (myRGBQUAD*)((myBYTE*)m_pDib+sizeof(myBITMAPFILEHEADER) + sizeof(myBITMAPINFOHEADER));
	}		
	return NULL;
}

myDWORD SZImageBase::GetPaletteSize() const
{
	return (head.biClrUsed * sizeof(myRGBQUAD));
}

bool SZImageBase::writeBMPFile(const char *fileName)
{
	if (NULL==m_pDib)
		return false;
	
	FILE *hFile = fopen(fileName, "wb");
	if(hFile)
	{
		fwrite(m_pDib,GetSize(),1,hFile);
		fflush(hFile);
		fclose(hFile);
		return true;
	}
	return false;
}
#ifdef USE_CXIMAGE
short SZImageBase::ntohs1(const short word)
{
	if (m_bLittleEndianHost) return word;
	return ( (word & 0xff) << 8 ) | ( (word >> 8) & 0xff );
}

long SZImageBase::ntohl1(const long dword)
{
	if (m_bLittleEndianHost) return dword;
	return  ((dword & 0xff) << 24 ) | ((dword & 0xff00) << 8 ) |
		((dword >> 8) & 0xff00) | ((dword >> 24) & 0xff);
} 
#else
short SZImageBase::ntohs(const short word)
{
	if (m_bLittleEndianHost) return word;
	return ( (word & 0xff) << 8 ) | ( (word >> 8) & 0xff );
}

long SZImageBase::ntohl(const long dword)
{
	if (m_bLittleEndianHost) return dword;
	return  ((dword & 0xff) << 24 ) | ((dword & 0xff00) << 8 ) |
		((dword >> 8) & 0xff00) | ((dword >> 24) & 0xff);
} 
#endif // USE_CXIMAGE
bool SZImageBit1::Init(int width, int height)
{ 	
 	if (width<=0||height<=0)
	{
		return false;
	} 
	if (m_pDib!=0)	
	{
		if (m_iWidth==width && m_iHeight==height)
		{
			return true;
		}
	}
	m_iWidth  = width;
	m_iHeight = height;
	
	if (!Create(m_iWidth,m_iHeight,1))  
		return false;
	InitGrayPalette();  
	return true;
}

bool SZImageBit4::Init(int width, int height)
{ 	 
	if (width<=0||height<=0)
	{
		return false;
	} 
	if (m_pDib!=0)	
	{
		if (m_iWidth==width && m_iHeight==height)
		{
			return true;
		}
	}
	m_iWidth  = width;
	m_iHeight = height;
	
	if (!Create(m_iWidth,m_iHeight,4))  
		return false;
	InitGrayPalette();  
	return true;
}

bool SZImageBit8::Init(int width, int height)
{  
	if (width<=0||height<=0)
	{
		return false;
	} 
	if (m_pDib!=0)	
	{
		if (m_iWidth==width && m_iHeight==height)
		{
			return true;
		}
	}
	m_iWidth  = width;
	m_iHeight = height;
	
	if (!Create(m_iWidth,m_iHeight,8))  
		return false;
	InitGrayPalette();  
	return true;
}

bool SZImageBit1::SetDesPixel(long x,long y,myBYTE c)
{	
	if ((m_pDib==NULL)||(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight))
		return false;

	myBYTE* iDst= m_pImageBits + y*m_dwEffWidth + (x*head.biBitCount >> 3);	
	myBYTE   pos= (myBYTE)(7-x%8);
	*iDst &= ~(0x01<<pos);
	*iDst |= ((c & 0x01)<<pos);	
	return true;
}

bool SZImageBit4::SetDesPixel(long x,long y,myBYTE c)
{	
	if ((m_pDib==NULL)||(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight))
		return false;

	myBYTE* iDst= m_pImageBits + y*m_dwEffWidth + (x*head.biBitCount >> 3);
	myBYTE pos  = (myBYTE)(4*(1-x%2));
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
	return true;
}

bool SZImageBit8::SetDesPixel(long x,long y,myBYTE c)
{
	if ((m_pDib==NULL)||(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight))
		return false;

	m_pImageBits[y*m_dwEffWidth + x]=c;	
	return true; 
}

myBYTE	SZImageBit1::GetOriPixelIndex(long x,long y)
{	 
	myBYTE pos=0;
	myBYTE rt=0;
	if (m_pDib==NULL)
		return rt;
	
	if (y<0 || y>=head.biHeight || x<0 || x>=head.biWidth)
	{
		x=max(x,0); x=min(x, m_iWidth-1);
		y=max(y,0); y=min(y, m_iHeight-1);			
	}
	
	myBYTE iDst=*(m_pImageBits +y*m_dwEffWidth + (x*head.biBitCount >> 3));
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
	return rt;
}

myBYTE	SZImageBit4::GetOriPixelIndex(long x,long y)
{	 
	myBYTE pos=0;
	myBYTE rt=0;
	if (m_pDib==NULL)
		return rt;
	
	if (y<0 || y>=head.biHeight || x<0 || x>=head.biWidth)
	{
		x=max(x,0); x=min(x, m_iWidth-1);
		y=max(y,0); y=min(y, m_iHeight-1);			
	}
	
	myBYTE iDst=*(m_pImageBits +y*m_dwEffWidth + (x*head.biBitCount >> 3));
 	pos = (myBYTE)(4*(1-x%2));
	iDst &= (0x0F<<pos);
  	rt  = (myBYTE)(iDst >> pos);
	return rt;
}

myBYTE	SZImageBit8::GetOriPixelIndex(long x,long y)
{	 
	myBYTE rt=0;
	if (m_pDib==NULL)
		return rt;
 
	if (y<0 || y>=head.biHeight || x<0 || x>=head.biWidth)
	{
		x=max(x,0); x=min(x, m_iWidth-1);
		y=max(y,0); y=min(y, m_iHeight-1);			
	}

  	myBYTE *iDst=m_pImageBits + y*m_dwEffWidth + x; 
  	rt = *iDst; 
	return rt;
}

myRGBQUAD	SZImageBit1::GetOriPixel(long x,long y)
{
	myRGBQUAD rgb={0,0,0,0};
	if (m_pDib==NULL)
		return rgb;
 
	myBYTE	index=GetOriPixelIndex(x, y);
	if (index<head.biClrUsed)
	{
		myRGBQUAD* pal  =GetPalette(index);	 
		if (pal)
		{
			rgb.rgbRed		=pal->rgbRed;		
			rgb.rgbGreen	=pal->rgbGreen;
			rgb.rgbBlue		=pal->rgbBlue;
		}	
	}
	return rgb;
}

SZImageBit24::SZImageBit24():SZImageBase()
{
	m_bSupportAlpha  =true;
}

SZImageBit24::~SZImageBit24()
{

}

bool SZImageBit24::Init(int width, int height)
{  
	if (width<=0||height<=0)
	{
		return false;
	} 
 
	if (m_pDib!=0)	
	{
		if (m_iWidth==width && m_iHeight==height)
		{
			return true;
		}
	}
	m_iWidth  = width;
	m_iHeight = height;
	m_bSupportAlpha =true;
 
	if (!Create(m_iWidth,m_iHeight,24))  
		return false;
 	InitGrayPalette(); 	
	return true;
}

myRGBQUAD	SZImageBit24::GetOriPixel(long x,long y)
{	
	myRGBQUAD rgb={0,0,0,0};
	if (m_pDib==NULL)
		return rgb;
	
	if (y<0 || y>=head.biHeight || x<0 || x>=head.biWidth)
	{
		x=max(x,0); x=min(x, m_iWidth-1);
		y=max(y,0); y=min(y, m_iHeight-1);			
	}
	
	myBYTE* iDst = m_pImageBits + y*m_dwEffWidth + x*3;
	rgb.rgbBlue = *iDst++;
	rgb.rgbGreen= *iDst++;
	rgb.rgbRed  = *iDst;
	return rgb;
}

bool SZImageBit24::SetDesPixel(long x,long y,myRGBQUAD i,myBYTE ialpha)
{
	if ((m_pDib==NULL)||(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight))
		return false;
	
	if (m_bSupportAlpha)
	{
		if (ialpha==0)
		{
			return true; 
		}
		else
		{ 
			myBYTE* iDst = m_pImageBits + y*m_dwEffWidth + x*3; 		
			if(ialpha==255)
			{
				*iDst++ =i.rgbBlue;
				*iDst++ =i.rgbGreen;
				*iDst   =i.rgbRed;	
			}else
			{
				myBYTE a1 = (myBYTE)~ialpha;			
				*iDst++ =(myBYTE)(((*iDst)*a1+i.rgbBlue*ialpha)/255); 
				*iDst++ =(myBYTE)(((*iDst)*a1+i.rgbGreen*ialpha)/255);
				*iDst   =(myBYTE)(((*iDst)*a1+i.rgbRed*ialpha)/255);	
			}
		}		
	}else
	{
		myBYTE* iDst = m_pImageBits + y*m_dwEffWidth + x*3; 		
		*iDst++ =i.rgbBlue;
		*iDst++ =i.rgbGreen;
		*iDst   =i.rgbRed;
	}
	return true;
}

int SZImageBit24::DrawLine(myPOINT start,myPOINT end,myRGBQUAD clr)
{ 
	int posmin,posmax;
	if (NULL==m_pDib)
		return -1;
	if (start.y==end.y)
	{
		if (start.x==end.x)
		{
			SetDesPixel(start.x,m_iHeight-1-start.y,clr,255);
			return 2;
		}
		if (start.x>end.x)
		{
			posmin=end.x;
			posmax=start.x;
		}else
		{
			posmin=start.x;
			posmax=end.x;
		}
		
		for (;posmin<=posmax;posmin++)
		{
			SetDesPixel(posmin,m_iHeight-1-start.y,clr,255);		
		}
		return 1;
	}
	
	
	if (start.x==end.x)
	{
		if (start.y==end.y)
		{
			SetDesPixel(start.x,m_iHeight-1-start.y,clr,255);
			return 2;
		}
		if (start.y>end.y)
		{
			posmin=end.y;
			posmax=start.y;
		}else
		{
			posmin=start.y;
			posmax=end.y;
		}
		
		for (;posmin<=posmax;posmin++)
		{
			SetDesPixel(start.x,m_iHeight-1-posmin,clr,255);		
		}
		return 1;
	}
	
	return 0;	  
} 

bool SZImageBit32::Init( int width, int height )
{
	if (width<=0||height<=0)
	{
		return false;
	} 
	
	if (m_pDib!=0)	
	{
		if (m_iWidth==width && m_iHeight==height)
		{
			return true;
		}
	}
	m_iWidth  = width;
	m_iHeight = height;
	m_bSupportAlpha =true;
	
	if (!Create(m_iWidth,m_iHeight,32))  
		return false;
	InitGrayPalette(); 	
	return true;
}

SZImageBit32::SZImageBit32():SZImageBase()
{
	m_bSupportAlpha  =true;
}

SZImageBit32::~SZImageBit32()
{
	
}

myRGBQUAD SZImageBit32::GetOriPixel( long x,long y )
{
	myRGBQUAD rgb={0,0,0,0};
	if (m_pDib==NULL)
		return rgb;
	
	if (y<0 || y>=head.biHeight || x<0 || x>=head.biWidth)
	{
		x=max(x,0); x=min(x, m_iWidth-1);
		y=max(y,0); y=min(y, m_iHeight-1);			
	}
	
	myBYTE* iDst = m_pImageBits + y*m_dwEffWidth + x*4;
	rgb.rgbBlue = *iDst++;
	rgb.rgbGreen= *iDst++;
	rgb.rgbRed  = *iDst++;
	rgb.rgbReserved = *iDst;
	return rgb;
}
