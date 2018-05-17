// SZImage.h: interface for the BWImageScalingAlgorithm class.
//
//////////////////////////////////////////////////////////////////////
/*
* Copyright (c) 2011,  北京超星研发部 
* All rights reserved.
*
* 文件名称：SZImage.h
* 文件标识：AFX_BWIMAGESCALINGALGORITHM_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_
* 摘 要：   简单位图处理类实现
*
* 当前版本：1.0
* 作 者：   邵振     shaozhen1@163.com
* 完成日期：2011年08月31日
*
*
*/
#if !defined(AFX_SZIMAGE_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_)
#define AFX_SZIMAGE_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_
#include "SZDefine.h"


typedef struct tagmyPOINT 
{
	int     x;
	int 	y; 
}myPOINT, *myLPOINT;

typedef struct tagmyBITMAPFILEHEADER 
{
	myWORD    bfType;
	myWORD	  bfSizelow;
	myWORD	  bfSizehigh;
	myWORD    bfReserved1;
	myWORD    bfReserved2;
	myWORD    bfOffBitslow;
	myWORD	  bfOffBitshigh;
}myBITMAPFILEHEADER,  *myLPBITMAPFILEHEADER, *myPBITMAPFILEHEADER;

typedef struct tagmyBITMAPINFOHEADER
{
	myDWORD      biSize;
	myDWORD		 biWidth;
	myDWORD      biHeight;
	myWORD       biPlanes;
	myWORD       biBitCount;
	myDWORD      biCompression;
	myDWORD      biSizeImage;
	myDWORD		 biXPelsPerMeter;
	myDWORD      biYPelsPerMeter;
	myDWORD      biClrUsed;
	myDWORD      biClrImportant;
}myBITMAPINFOHEADER,   *myLPBITMAPINFOHEADER, *myPBITMAPINFOHEADER;

typedef struct tagmyRGBQUAD 
{
	myBYTE    rgbBlue;
	myBYTE    rgbGreen;
	myBYTE    rgbRed;
	myBYTE    rgbReserved;
}myRGBQUAD,* myLPRGBQUAD;

class SZImageBase  
{
public:
	SZImageBase();
	virtual ~SZImageBase();
	virtual bool	    Init(int width, int height) = 0;
		void		Destroy();
	virtual myRGBQUAD	GetOriPixel(long x,long y)  = 0;
	bool        writeBMPFile(const char *fileName);
	int			GetWidth()const{return m_iWidth;}
	int			GetHeight()const{return m_iHeight;}
	int			GetEffWidth()const{return m_dwEffWidth;}
	myDWORD		GetPaletteNum()const{return head.biClrUsed;}
	myDWORD		GetPaletteSize()const;
	int			GetBitCount()const;
	long		GetSize() const;
	void*		GetBMP() const;	
	void*		GetDIB() const;	
	myBYTE*		GetBits(myDWORD row = 0)const;	
	void		SetBits(myBYTE* pdata,myDWORD offset,myDWORD size);
	bool		SetGrayPalette(myDWORD index,myRGBQUAD color);
#ifdef USE_CXIMAGE
	short		ntohs1 (const short word);
	long		ntohl1 (const long dword);
#else
	short		ntohs(const short word);
	long		ntohl(const long dword); 
#endif // USE_CXIMAGE
	static 	myRGBQUAD   MakeRGBQuad(myBYTE r,myBYTE g,myBYTE b);
	static 	myRGBQUAD   MakeRGBQuad(myDWORD color);
protected:
	void*		Create(myDWORD dwWidth, myDWORD dwHeight, myBYTE wBpp);

	void		InitGrayPalette();	
	myRGBQUAD*  GetPalette(myDWORD index = 0) const;	
	myBITMAPINFOHEADER    head;   //位图头 
	void*				  m_pDib; //包含 (文件头,位图头, 调色板, 像素)
	myBYTE*				  m_pImageBits;     
	myDWORD				  m_dwEffWidth;

	bool				  m_bLittleEndianHost;	

public:
	int					  m_iWidth;
	int					  m_iHeight;
};

class SZImageBit1: public SZImageBase
{
public:
	virtual bool		Init(int width, int height);
	virtual myBYTE		GetOriPixelIndex(long x,long y);
	virtual myRGBQUAD	GetOriPixel(long x,long y);
	bool				SetDesPixel(long x,long y,myBYTE c);
};

class SZImageBit4: public SZImageBit1
{
public:
	virtual bool		Init(int width, int height);
	virtual myBYTE		GetOriPixelIndex(long x,long y);
 //	virtual myRGBQUAD	GetOriPixel(long x,long y);
	bool				SetDesPixel(long x,long y,myBYTE c);
};

class SZImageBit8: public SZImageBit1
{
public:
	virtual bool		Init(int width, int height);
	virtual myBYTE		GetOriPixelIndex(long x,long y);
// 	virtual myRGBQUAD	GetOriPixel(long x,long y);
	bool				SetDesPixel(long x,long y,myBYTE c);
};

class SZImageBit24: public SZImageBase
{
public:
	SZImageBit24(); 
	~SZImageBit24();
	virtual bool	    Init(int width, int height);
	myRGBQUAD	 	    GetOriPixel(long x,long y);
	bool			    SetDesPixel(long x,long y,myRGBQUAD color,myBYTE ialpha);  
	int					DrawLine(myPOINT start,myPOINT end,myRGBQUAD clr);
	bool				IsSupportAlpha()const{return m_bSupportAlpha;}
	void				SetSupportAlpha(bool bSupportAlpha){m_bSupportAlpha=bSupportAlpha;}
private:
	bool				m_bSupportAlpha;	
};
 

class SZImageBit32: public SZImageBase
{
public:
	SZImageBit32(); 
	~SZImageBit32();
	virtual bool	    Init(int width, int height);
	myRGBQUAD	 	    GetOriPixel(long x,long y);
private:
	bool				m_bSupportAlpha;	
};
#endif // !defined(AFX_SZIMAGE_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_)

