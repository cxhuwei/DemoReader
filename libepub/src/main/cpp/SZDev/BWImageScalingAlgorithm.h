// BWImageScalingAlgorithm.h: interface for the BWImageScalingAlgorithm class.
//
//////////////////////////////////////////////////////////////////////
/*
* Copyright (c) 2011,  ���������з��� 
* All rights reserved.
*
* �ļ����ƣ�BWImageScalingAlgorithm.h
* �ļ���ʶ��AFX_BWIMAGESCALINGALGORITHM_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_
* ժ Ҫ��   2ֵλͼ��С�㷨ʵ��
*
* ��ǰ�汾��1.0
* �� �ߣ�   ����     shaozhen1@163.com
* ������ڣ�2011��07��26��
*
* �޸ļ�¼ : 
* �� ��        �汾     �޸���              
* 2011/08/02   1.1      ����                
* �޸�����
* 1.�����˿������ͼ���ÿ����ռ��λ��bpp
* 2.�Ż��˲��ִ���ṹ��ʹЧ�����
* 3.�¶�����my64long�������ͺ� tagmyBITMAPFILEHEADER���ݽṹ�����������ӿ�ֱ�����bmp�ļ�
* 4.��Ƶ��ʹ�ö�����λ�����Ĵ����û��ʵ�֣����ú궨��ȷ�����뻷�����ж��Ƿ��û��ʵ��
* 5.Ϊ�����ƶ�ƽ̨��ARM�������ϸĺõ�����ٶȺ�Ч�ʣ���ʹ�ø�������Ĵ���������������ʵ�֣�
*   Ϊ�����׼ȷ���ȣ���10^10 ���Ŵ����,�����ӿ�DoZoom1��������֮,������ԭ�нӿ�DoZoom()
*
*
* �޸ļ�¼ : 
* �� ��        �汾     �޸���              
* 2011/08/12   1.2      ����                
* �޸�����
* 1.��������С24λ���ͼ�Ĺ��ܣ�Ϊ�˼����жϲ�����ԭ�����룬��ԭ��BWImageScalingAlgorithm����
* Bit24ImageScalingAlgorithm ����ʵ��
* 2.�ڲ��ԵĹ����з��������Ͳ���ģ�⸡������ʱ����bug������ĳЩͼ������ʱ������������ͨ��
* �о���������������µ���������
*
*
* �޸ļ�¼ : 
* �� ��        �汾     �޸���              
* 2011/08/17   1.3      ����                
* �޸�����
* 1.��������С8λ�Ҷ�ͼ�Ĺ���  
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
	//��ʼ���ӿ�,inbuf ����С��2ֵͼ��buffer
	//bits      ÿ����λ��������ֻ��Ϊ1��ֻ��2ֵͼ������(Ԥ���ӿڣ�Ŀǰ��Ϊ1���ᴦ������Ҳ���ʵ��4,8,16,24) 
	//Oriwidth  ԭͼ���
	//Oriheight ԭͼ�߶�
	//Deswidth  ���ͼ���
	//Desheight ���ͼ�߶�
	//type =0	�ȶ�����Deswidth    DesheightΪ���ͼ����
	//type =1	����Deswidth �����,Desheight����ݱ仯�������
	//type =2	����Desheight�����,Deswidth����ݱ仯�������
	//outImgBitPerPixel �������ͼ���ÿ����ռ��λ����Ĭ����4λ16ɫ
	virtual bool		DoZoom(bool quick);//true �Ͽ죬Ч�����Խ��� false ��΢��Щ����Ч������һЩ
	virtual bool		DoZoom1(bool quick);//������ģ�⸡�����㣬����Ч��DoZoom,������arm���ٶ��ܹ���һЩ
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

	void*				  m_pOriBuffer;//������ͼ��buffer
	void*				  m_pDib; //���� (�ļ�ͷ,λͼͷ, ��ɫ��, ����)
	myBYTE*				  m_pImageBits;
    myBITMAPINFOHEADER    head;   //λͼͷ 
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
	//��ʼ���ӿ�,inbuf ����С��8λ�Ҷ�ͼ��buffer		 
private: 
	virtual myBYTE		GetOriPixelGray(long x,long y);
};

class Bit24ImageScalingAlgorithm : public BWImageScalingAlgorithm
{
public:
	virtual bool	 Init(void *inbuf, int Oriwidth, int Oriheight,	int Deswidth, int Desheight,int Optype);	
	//��ʼ���ӿ�,inbuf ����С��24ֵͼ��buffer	  
	virtual bool	DoZoom(bool quick);//true �Ͽ죬Ч�����Խ��� false ��΢��Щ����Ч������һЩ
	virtual bool	DoZoom1(bool quick);//������ģ�⸡�����㣬����Ч��DoZoom,������arm���ٶ��ܹ���һЩ
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

