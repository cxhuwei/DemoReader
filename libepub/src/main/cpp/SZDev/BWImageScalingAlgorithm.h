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
#if !defined(AFX_BWIMAGESCALINGALGORITHM_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_)
#define AFX_BWIMAGESCALINGALGORITHM_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_
#include "SZDefine.h"
#include "SZImage.h"
class BWImageScalingAlgorithm  
{
public:
	BWImageScalingAlgorithm();
	virtual ~BWImageScalingAlgorithm();
	virtual bool	 Init(void *inbuf, int Oriwidth, int Oriheight, int bits, bool bB0W1,
			int Deswidth, int Desheight,int Optype,int outImgBitPerPixel=4);
	virtual bool	 Init(void *inbuf, int Oriwidth, int Oriheight,	int Deswidth, int Desheight,int Optype);
	//初始化接口,inbuf 待缩小的2值图像buffer
	//bits      每像素位数，现在只能为1，只做2值图像缩放(预留接口，目前不为1不会处理，将来也许会实现4,8,16,24) 
	//Oriwidth  原图宽度
	//Oriheight 原图高度
	//Deswidth  输出图宽度
	//Desheight 输出图高度
	//type =0	既定按照Deswidth    Desheight为输出图运算
	//type =1	按照Deswidth 来输出,Desheight会根据变化比率算出
	//type =2	按照Desheight来输出,Deswidth会根据变化比率算出
	//outImgBitPerPixel 设置输出图像的每像素占用位数，默认是4位16色
	virtual bool		DoZoom(bool quick);//true 较快，效果可以接受 false 稍微慢些，但效果更好一些
	virtual bool		DoZoom1(bool quick);//用整形模拟浮点运算，近似效果DoZoom,可能在arm上速度能够快一些
	int			GetDstWidth()const;
	int			GetDstHeight()const;
	int			GetDstEffWidth()const;
	void*		GetDIB() const;	
	void*		GetBMP() const;	
	myBYTE*		GetBits(myDWORD row = 0);
	long		GetSize();
	long		GetImageSize();
	myDWORD		GetPaletteSize();
protected:
	void*		Create(myDWORD dwWidth, myDWORD dwHeight, myBYTE wBpp);
	void		Destroy();
	myBYTE		GetAreaColorInterpolated1(float const xc, float const yc, float const w, float const h);	
	myBYTE		GetAreaColorInterpolated2(float const xc, float const yc, float const w, float const h);
	myBYTE		GetAreaColorInterpolated11(myDWORD const xc, myDWORD const yc, myDWORD const w, myDWORD const h);	
	myBYTE		GetAreaColorInterpolated22(int const xc, int const yc, int const w, int const h);
	myBYTE		GetNearestIndex(myBYTE c);
	virtual myBYTE		GetOriPixelGray(long x,long y);
	bool		SetDesPixelIndex(long x,long y,myBYTE i);
	myRGBQUAD*  GetPalette() const;
	void		SetGrayPalette();	

	void*				  m_pOriBuffer;//待处理图像buffer
	void*				  m_pDib; //包含 (文件头,位图头, 调色板, 像素)
	myBYTE*				  m_pImageBits;
    myBITMAPINFOHEADER    head;   //位图头 
	myDWORD				  dwEffWidth;
	myDWORD				  m_dwOriEffWidth;
	bool				  m_bB0W1;
	int					  m_outImgBitPerPixel;	
	int					  m_iOriwidth;
	int					  m_iOriheight;
	int					  m_iDeswidth;
	int					  m_iDesheight;
	int					  m_iOptype;
	int					  m_ibits;
};

class Bit8ImageScalingAlgorithm : public BWImageScalingAlgorithm
{
public:
	virtual bool	 Init(void *inbuf, int Oriwidth, int Oriheight,	int Deswidth, int Desheight,int Optype);
	//初始化接口,inbuf 待缩小的8位灰度图像buffer		 
private: 
	virtual myBYTE		GetOriPixelGray(long x,long y);
};

class Bit24ImageScalingAlgorithm : public BWImageScalingAlgorithm
{
public:
	virtual bool	 Init(void *inbuf, int Oriwidth, int Oriheight,	int Deswidth, int Desheight,int Optype);	
	//初始化接口,inbuf 待缩小的24值图像buffer	  
	virtual bool	DoZoom(bool quick);//true 较快，效果可以接受 false 稍微慢些，但效果更好一些
	virtual bool	DoZoom1(bool quick);//用整形模拟浮点运算，近似效果DoZoom,可能在arm上速度能够快一些
private:
	myRGBQUAD		GetAreaColorInterpolated1(float const xc, float const yc, float const w, float const h);	
	myRGBQUAD		GetAreaColorInterpolated2(float const xc, float const yc, float const w, float const h);
	myRGBQUAD		GetAreaColorInterpolated11(myDWORD const xc, myDWORD const yc, myDWORD const w, myDWORD const h);	
	myRGBQUAD		GetAreaColorInterpolated22(int const xc, int const yc, int const w, int const h);
	myRGBQUAD		GetOriPixel(long x,long y);
	bool			SetDesPixel(long x,long y,myRGBQUAD color); 
	static	void	AddAveragingCont(myRGBQUAD const &color, float const surf, float &rr, float &gg, float &bb);
};

#endif // !defined(AFX_BWIMAGESCALINGALGORITHM_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_)

