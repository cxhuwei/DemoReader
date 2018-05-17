#include "stdafx.h"
#include "Render.h"
#include "MAlgorithm.h"

#include <android/log.h>
#define LOG_TAG "PDGPARSER"
#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)


const float g_Paragraph = 2.5;//段间距是行间距的倍数
CRender::CRender(void)
{
	m_PrevWordCharIndex = 0;
	m_NowWordCharIndex = 0;
	m_nMrginLeft = 50;
	m_nMrginRight = 50;
	m_nMrginTop = 50;
	m_nMrginBottom = 50;
	m_nTabSize = 2;//缩进（2个字的宽度）
	m_nWordSpac = 1;
	m_nVertSpac = 0.6;
    m_nParagraphSpac = 0.6;//m_nVertSpac * g_Paragraph;

	m_clrForeG = SZImageBase::MakeRGBQuad(0,0,0);


	m_PreWordinfo.x = m_nMrginLeft;
	m_PreWordinfo.y = m_nMrginTop;
	m_iStop = false;
    m_curLineMaxHeight = 0;
}

CRender::CRender( const CRender& render )
{
	this->m_clrForeG = render.m_clrForeG;
	this->m_nTabSize = render.m_nTabSize; 
	this->m_nVertSpac = render.m_nVertSpac; 
	this->m_nParagraphSpac = render.m_nParagraphSpac;
	this->m_nWordSpac = render.m_nWordSpac; 
	this->m_nMrginLeft = render.m_nMrginLeft;  
	this->m_nMrginRight = render.m_nMrginRight; 
	this->m_nMrginTop = render.m_nMrginTop;  
	this->m_nMrginBottom = render.m_nMrginBottom; 
	this->m_OutDevW = render.m_OutDevW; 
	this->m_OutDevH = render.m_OutDevH;
	this->m_OffsetX = render.m_OffsetX; 
	this->m_OffsetY = render.m_OffsetY;	
	this->m_PreWordinfo.x = m_nMrginLeft;
	this->m_PreWordinfo.y = m_nMrginTop;
	this->m_iStop = render.m_iStop;

	m_curLineMaxHeight = 0;
}

CRender::~CRender(void)
{
}

void CRender::Init( int iWidth, int iHeight,myDWORD nFormat,myRGBQUAD clrFG )
{
	m_OutDevW	   = iWidth;
	m_OutDevH	   = iHeight;
	m_clrForeG	   = clrFG;

	m_OffsetX	   = m_nMrginLeft; 
	m_OffsetY	   = m_nMrginTop; 

}

/* 
	beginPos ：该content中，从beginPos开始绘制
	endpos: 返回给上层，该content中，绘制的最后一个位置
	endpos :-2 该content绘制完了
	endpos :-1 该content还没有绘制，offsety已经超过范围了。

*/
int CRender::DrawContentText( SZImageBit24& image,SZFTFont* pChiFont,SZFTFont* pEngFont,int contentId,Content* content,int beginPos,int& endPos ,pageWordinfo* pwi,Vt_curPageMultiMedia* cpmm,bool isRender)
{
	if (pChiFont == NULL || pEngFont == NULL || content == NULL)
	{
		return -1;
	}
	int nCFullW=pChiFont->GetFullWidthAdvanceCH();//中文整字宽度
	int nCHalfW=pChiFont->GetHalfWidthAdvanceCH();//中文半字宽度
	int nEFullW=pEngFont->GetFullWidthAdvance();//英文整字宽度
	int nEHalfW=pEngFont->GetHalfWidthAdvance();//英文整字宽度

	//把content中的文本转成到一个字符串中
	std::wstring strText = L"";
	std::wstring strLineText = L"";
	if (content->before != NULL)
	{
		strText += content->before;
	}
	if (content->text != NULL)
	{
		strText += content->text;
	}

	if (content->isP_start && content->textStyle.textAlign == LEFT)
	{
		if (content->before == NULL && beginPos == 0)
		{//如果段落前面本身没有缩进，那么手动增加缩进
			//strText.insert(0,L"\t");//开始增加一个制表符
			m_OffsetX += ((nCFullW + m_nWordSpac) * m_nTabSize);
		}
		if (content->before!= NULL)
		{
			if (myWcslen(content->before) == beginPos)
			{
				m_OffsetX += ((nCFullW + m_nWordSpac) * m_nTabSize);
			}
		}
	}
	if(content->isP_end)
	{
		strText += L"\n";
	}

	//开始绘制

	//样式
	if (content->isHaveStyle)
	{
		pChiFont->SetIsBold(content->textStyle.Bold);
		pChiFont->SetIsItalic(content->textStyle.isItalic);
		pEngFont->SetIsBold(content->textStyle.Bold);
		pEngFont->SetIsItalic(content->textStyle.isItalic);
	}

	int drawX = m_OffsetX;//m_offsetX记录了上次绘制的横坐标
	//int drawY = m_OffsetY;//m_offsetY记录了上次绘制的纵坐标

	int textLength,blankLenght;
	int tempPrevWordIndex = m_PrevWordCharIndex;
	int deleteCount(0);//最终需要删除的word个数
	if ((m_OffsetY + nCFullW)> (m_OutDevH - m_nMrginBottom))
	{
		endPos--;
		m_OffsetX = m_nMrginLeft;
		m_OffsetY = m_nMrginTop;
		return 0;
	}
	//第一步，统计一行能放得下的所有字
	int offset = beginPos;
	wchar_t temp;
	for (endPos = beginPos; endPos < strText.length(); endPos++)
	{
		if (nEFullW > m_curLineMaxHeight)
		{
			m_curLineMaxHeight = nEFullW;
		}

		if (m_iStop)
		{
			break;
		}
		myWORD nowWchar = strText[endPos];
		temp = strText[endPos];
		if (nowWchar == 0x0a || nowWchar == 0x0d)
		{//主动换行
			//strLineText += nowWchar;
			if (strLineText.length() > 0 && !isRender)
			{
				if (content->textStyle.textAlign == CENTER && m_OffsetX == m_nMrginLeft)
				{
					textLength = drawX - m_OffsetX;
					blankLenght = m_OutDevW - m_nMrginLeft - m_nMrginRight;
					int realDrawX = (blankLenght - textLength) / 2 + m_nMrginLeft;
					m_OffsetX = realDrawX;
				}
				if (content->textStyle.textAlign == RIGHT && m_OffsetX == m_nMrginLeft)
				{
					textLength = drawX - m_OffsetX;
					blankLenght = m_OutDevW - m_nMrginLeft - m_nMrginRight;
					int realDrawX = m_nMrginLeft +	blankLenght - textLength;
					m_OffsetX = realDrawX;
				}
				m_PrevWordCharIndex = tempPrevWordIndex;
				
				//drawline
				DrawLine(image,pChiFont,pEngFont,m_OffsetX,m_OffsetY,strLineText,contentId,content,offset,pwi,cpmm,true,false);
				offset = endPos;
				strLineText = L"";
			}
			//把绘制坐标移动到下一行
			drawX = m_OffsetX = m_nMrginLeft;
			m_OffsetY += (m_curLineMaxHeight/*nEFullW*/ + m_nParagraphSpac*nCFullW/*m_nVertSpac * g_Paragraph*/);//段落之间用2倍行距做段间距
			m_curLineMaxHeight = 0;
			if (m_OffsetY > (m_OutDevH - m_nMrginBottom - nEFullW))
			{//该页绘制满了
				m_OffsetY = m_nMrginTop;
				m_OffsetX = m_nMrginLeft;
				if (endPos == strText.length() - 1)
				{//最后一个
					endPos = -2;
				}
				return 0;

			}
		}
		else
		{
			if (nowWchar == 0x20)//L' '
			{
				drawX += (nCHalfW + m_nWordSpac);
			}
			else if (nowWchar == 0x3000)//L'　'
			{
				drawX += (nCFullW + m_nWordSpac);
			}
			else if (nowWchar == 0x9)//L'\t'
			{
				drawX += ((nCFullW + m_nWordSpac) * m_nTabSize);
			}
			else
			{
				if (isEngChar(nowWchar))
				{
					pEngFont->BuildCharCache(nowWchar);
					CharCacheNode* temp = pEngFont->GetCharCacheNode(nowWchar);
					m_NowWordCharIndex = temp->charGlyphIndex;
					drawX += pEngFont->GetTwoCharKerning(m_PrevWordCharIndex,m_NowWordCharIndex);
					drawX += temp->w/*charAdvance*/;
					m_PrevWordCharIndex = m_NowWordCharIndex;
				}
				else
				{
					int CLR = returnCharLeftRight(nowWchar);
						pChiFont->BuildCharCache(nowWchar);
						CharCacheNode* pNode = pChiFont->GetCharCacheNode(nowWchar);
						if (pNode == NULL)
						{
							drawX += nCFullW;
						}
						else
						{
						int BW(0),lk(0),rk(0);
						//规则:标点占字宽的2/3。左标点居左占剩余空白的1/4,居右占3/4。右标点跟左标点相反
						if (CLR < 3)
						{
							BW = CalcFloat(pNode->charAdvance,(float)2/3);
							// 右标点需要在画之前进行调整
							float times = (CLR == 1)?((float)1/4):((float)3/4);
							lk = CalcFloat(BW - pNode->w,times);
							rk = BW - pNode->w - lk;
							drawX += lk;
						}
							m_NowWordCharIndex = pNode->charGlyphIndex;
							drawX += pChiFont->GetTwoCharKerning(m_PrevWordCharIndex,m_NowWordCharIndex);
						drawX += (CLR == 4?pNode->charAdvance:pNode->w)/*charAdvance*/;
							drawX += m_nWordSpac;
						drawX += rk;
// 						if (CLR == 1)
// 						{//左边标点
// 							drawX += pNode->x;//把实际标点距离左边框距离增加到标点之后，增加与下一个字的距离
// 						}
							
						}
					m_PrevWordCharIndex = m_NowWordCharIndex;
				}
			}

			if (drawX > (m_OutDevW - m_nMrginRight))
			{//行绘制到末尾了，需要换行了
				if (content->textStyle.textAlign == CENTER && m_OffsetX == m_nMrginLeft)
				{
					textLength = drawX - m_OffsetX;
					blankLenght = m_OutDevW - m_nMrginLeft - m_nMrginRight;
					int realDrawX = (blankLenght - textLength) / 2 + m_nMrginLeft;
					drawX = realDrawX;
				}
				if (content->textStyle.textAlign == RIGHT && m_OffsetX == m_nMrginLeft)
				{
					textLength = drawX - m_OffsetX;
					blankLenght = m_OutDevW - m_nMrginLeft - m_nMrginRight;
                    
                    if (blankLenght > textLength) {
                        int realDrawX = m_nMrginLeft +    blankLenght - textLength;
                        m_OffsetX = realDrawX;
                    }
				}
				m_PrevWordCharIndex = tempPrevWordIndex;

				if(nowWchar == 0x20 || nowWchar == 0x3000)
				{
					goto drawline;
				}
				//针对标点进行行字调整
				
				// 				int overage = m_OutDevW-m_nMrginRight - lastWordDrawX;
				Punctuation(strText,endPos,strLineText,deleteCount);
                if (deleteCount > 0) {
                    endPos -= deleteCount;
                }
				
				//drawline
drawline:		if (!isRender)
				{
					DrawLine(image,pChiFont,pEngFont,m_OffsetX,m_OffsetY,strLineText,contentId,content,offset,pwi,cpmm, deleteCount!= -1);
					offset = endPos;
				}
				
				endPos--;
				
				//把绘制坐标移动到下一行
				drawX = m_OffsetX = m_nMrginLeft;
				m_OffsetY += (m_curLineMaxHeight/*nEFullW*/ + m_nVertSpac*nCFullW);
				m_curLineMaxHeight = 0;
				if (m_OffsetY > (m_OutDevH - m_nMrginBottom - nEFullW))
				{//该页绘制满了
					m_OffsetY = m_nMrginTop;
					m_OffsetX = m_nMrginLeft;
					return 0;

				}
				strLineText = L"";

			}               
			else
			{
				strLineText += nowWchar;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
			}
		}
	}
	if (m_iStop)
	{
		return 1;
	}
	if (strLineText.length() > 0)
	{
		m_PrevWordCharIndex = tempPrevWordIndex;
		//当前content中剩余的
		if (isRender)
		{
			m_OffsetX = drawX;
		//	m_OffsetY = drawY;
		}
		else
		{
			DrawLine(image,pChiFont,pEngFont,m_OffsetX,m_OffsetY,strLineText,contentId,content,offset,pwi,cpmm,true,false);
			offset = endPos;
		}
	}
	endPos = -2;//标记该content全部绘制完
	return 1;
}


void CRender::Punctuation( std::wstring strFull,int index,std::wstring& strLine,int& deleteCount)
{
	//index  已经超过边界的那个字符的索引
	myWORD nowWchar;
	myWORD beforWchar;
	wstring temp = strLine;
	deleteCount = 0;//最后超过边界的那个字符，没有放入到strLine中
	if (index <= 0 || index >= strFull.size())
	{
		return;
	}
	nowWchar = strFull[index];

	//超过范围的最后一个是英文 处理方法，把前面一个字母也删掉，加一个横杠
	if (returnCharLeftRight(nowWchar) >= 3)
	{
		if (index - 1 > 0)
		{
			if (isEngChar(strFull[index - 1]))
			{
				strLine.append(L"-");
                deleteCount = -1;// mark "-" char
				return;
			}
		}
	}
	if(returnCharType(nowWchar) == 1)
	{//句末标点;句末标点不能出现在行首，所以需要把前一个字符跟当前字符一起放到下一行
		deleteCount++;
		strLine.erase(strLine.size()-1,1);
		if (strLine == L"")
		{
			return;
		}
		beforWchar = strFull[index - deleteCount];
		while(returnCharType(beforWchar) == 1)
		{
			if (deleteCount > 4)
			{
				return;
			}

			if (index - deleteCount >= 0)
			{
				deleteCount++;
				strLine.erase(strLine.size()-1,1);
				if (strLine == L"")
				{
					return;
				}
                beforWchar = strFull[index - deleteCount];
			}
			else
			{
				return;
			}
		}
	}
	
	//检测串最后一个字符是不是句首标点
	
	index = strLine.size();
	if (index == 0)
	{
		return;
	}
	nowWchar = strLine[strLine.size() - 1];
	while(returnCharType(nowWchar) == 2)
	{
		deleteCount++;
		strLine.erase(strLine.size()-1,1);
		if (deleteCount > 4)
		{
			return;
		}
		if (strLine.size() == 0)
		{
			return;
		}
		nowWchar = strLine[strLine.size() - 1];
	}


}
/*
	drawX:绘制该行的起始横坐标
	drawY:绘制该行的起始纵坐标
	strX:该行绘制的右端横坐标
	wordCountInstr:字符串中字的个数
*/
int CRender::DrawLine( SZImageBit24& image,SZFTFont* pChiFont,SZFTFont* pEngFont,int& drawX,int& drawY
					  ,std::wstring str,int contentId,Content* content ,int beginPos,pageWordinfo* pwi
					  ,Vt_curPageMultiMedia* cpmm,bool lastCharisReally, bool isNeedJustify)
{
	if (pChiFont == NULL || pEngFont == NULL || content == NULL)
	{
		return -1;
	}
	if (drawX < 0 || drawX > m_OutDevW || drawY < 0 || drawY > m_OutDevH)
	{
		return -2;
	}
	if (str.length() == 0)
	{
		return -3;
	}

	int nCFullW=pChiFont->GetFullWidthAdvanceCH();//中文整字宽度
	int nCHalfW=pChiFont->GetHalfWidthAdvanceCH();//中文半字宽度
	int nEFullW=pEngFont->GetFullWidthAdvance();//英文整字宽度
	int nEHalfW=pEngFont->GetHalfWidthAdvance();//英文整字宽度

	//两端对齐，预排版
	int beginX = drawX;
	int countLength = m_OffsetX;
	int zijianju = m_nWordSpac;
	int tianchong(0);//平均均摊到前n个字间距一个像素
	bool lastWordIsEng(false);
	int lineWordCount = 0;//记录一行里面的文字个数(不包括空格，制表符，换行符,英文单词算一个)
	int tempPrevWordIndex = m_PrevWordCharIndex;
	bool lineTopKong = true;//行首的空格去掉
	if (!(str[str.length() - 1] == L'\n' || !isNeedJustify))//1。str有换行符不需要两端对齐；2.主动标记不需要对齐
	{
		//主动换行不需要两端对齐
		for(int i = 0; i < str.length(); i++)
		{
			if (m_iStop)
			{
				return 1;
			}
			myWORD nowWchar = str[i];
			switch (nowWchar)
			{
			case 0x20:		 //L' '
// 				if (lineTopKong)
// 				{
// 					break;
// 				}
				countLength += (nCHalfW + m_nWordSpac);
				if (lastWordIsEng)
				{
					lineWordCount++;
					lastWordIsEng = false;
				}
				break;			
			case 0x3000	:	//L'　'
// 				if (lineTopKong)
// 				{
// 					break;
// 				}
				countLength += (nCFullW + m_nWordSpac);
				if (lastWordIsEng)
				{
					lineWordCount++;
					lastWordIsEng = false;
				}
				break;			
			case 0x9 :		//L'\t'
				countLength += ((nCFullW + m_nWordSpac) * m_nTabSize);
				if (lastWordIsEng)
				{
					lineWordCount++;
					lastWordIsEng = false;
				}
				lineTopKong = false;
				break;		
			default:
				lineTopKong = false;
				if (isEngChar(nowWchar))
				{
					lastWordIsEng = true;
					pEngFont->BuildCharCache(nowWchar);
					CharCacheNode* temp = pEngFont->GetCharCacheNode(nowWchar);
					m_NowWordCharIndex = pEngFont->GetCharCacheNode(nowWchar)->charGlyphIndex;
					countLength += pEngFont->GetTwoCharKerning(tempPrevWordIndex,m_NowWordCharIndex);
					countLength += temp->w/*charAdvance*/;
					tempPrevWordIndex = m_NowWordCharIndex;
				}
				else
				{
					lastWordIsEng = false;
					int CLR = returnCharLeftRight(nowWchar);
					pChiFont->BuildCharCache(nowWchar);
					CharCacheNode* pNode = pChiFont->GetCharCacheNode(nowWchar);
					if (pNode == NULL)
					{
						countLength += nCFullW;
					}
					else
					{
						int BW(0),lk(0),rk(0);
						//规则:标点占字宽的2/3。左标点居左占剩余空白的1/4,居右占3/4。右标点跟左标点相反
						if (CLR < 3)
						{
							BW = CalcFloat(pNode->charAdvance,(float)2/3);
							// 右标点需要在画之前进行调整
							float times = (CLR == 1)?((float)1/4):((float)3/4);
							lk = CalcFloat(BW - pNode->w,times);
							rk = BW - pNode->w - lk;
							countLength += lk;
						}
						m_NowWordCharIndex = pNode->charGlyphIndex;
						countLength += pChiFont->GetTwoCharKerning(m_PrevWordCharIndex,m_NowWordCharIndex);
						countLength += (CLR == 4?pNode->charAdvance:pNode->w);
						countLength += m_nWordSpac;
						countLength += rk;
// 						if (CLR == 1)
// 						{//左边标点
// 							countLength += pNode->x;//把实际标点距离左边框距离增加到标点之后，增加与下一个字的?嗬?
// 						}
						tempPrevWordIndex = m_NowWordCharIndex;
					}
					lineWordCount++;
				}

				break;
			}
		}
		if (lastWordIsEng)
		{
			lineWordCount++;
		}

		int shengyu = m_OutDevW - m_nMrginRight - countLength;
		if (lineWordCount > 1)
		{
			zijianju += shengyu / (lineWordCount - 1);
			tianchong = shengyu % (lineWordCount - 1);
		}
	}
	
	MultimediaPage mp;

	myRGBQUAD fontColor = m_clrForeG;//字体颜色
	if (content->isHaveStyle && content->textStyle.isFontColor)
	{
//        if (content->textStyle.fontColor.rgbRed != 0 || content->textStyle.fontColor.rgbGreen != 0|| content->textStyle.fontColor.rgbBlue != 0) {
//            fontColor.rgbBlue = content->textStyle.fontColor.rgbBlue;
//            fontColor.rgbGreen = content->textStyle.fontColor.rgbGreen;
//            fontColor.rgbRed = content->textStyle.fontColor.rgbRed;
//        }
        fontColor.rgbRed = (content->textStyle.fontColor.rgbRed + m_clrForeG.rgbRed)%256;
        fontColor.rgbGreen = (content->textStyle.fontColor.rgbGreen + m_clrForeG.rgbGreen)%256;
        fontColor.rgbBlue = (content->textStyle.fontColor.rgbBlue + m_clrForeG.rgbBlue)%256;
	}

	int tempDrawY = drawY;
    int underlineHeight = 0;
	if (content->contentType & CT_SUP)
	{//上标 纵坐标往上移动半个字高
		tempDrawY -= nCHalfW;
	}
	if (content->contentType & CT_Link)
	{//链接 如果没有定义链接自己颜色，就使用默认颜色
		if (!content->isHaveStyle || (content->isHaveStyle && !content->textStyle.isFontColor))
		{
            fontColor.rgbBlue = 255;
            fontColor.rgbGreen = 0;
            fontColor.rgbRed = 0;
		}
		mp.type = MT_Link;
		mp.lefttop_x = m_OffsetX;
		mp.lefttop_y = tempDrawY;
		mp.rightbottom_y = tempDrawY + nCFullW;
		int len = content->filepath.length();
		mp.filePath = content->filepath;
// 		mp->dataSize = len;
// 		mp->data = new myBYTE[len + 1];
// 		strcpy((char*)mp->data,content->filepath.c_str());
	}
	//开始画

	if (drawY > m_PreWordinfo.y + nCFullW)
	{
		if (pwi->lines[pwi->lines.size() - 1].words.size() > 0)
		{
			InitLineInfo();
			pwi->lines.push_back(m_curlineinfo);
		}
	}


	CharCacheNode* pNode = NULL;
	/*bool lastWordIsEng(false);*/
	lineTopKong = true;
	for(int i = 0; i < str.length(); i++)
	{
		if (m_iStop)
		{
			return 1;
		}
		myWORD nowWchar = str[i];

		wordinfo word;
		word.titleType = content->titleType;
		word.textStyle = content->textStyle;
		word.lineIndex = pwi->lines.size();
		word.WordUnicod = nowWchar;
		word.x = drawX;
		word.y = drawY;
		word.height = nCFullW;
        word.isRealChar = true;
        if (i == str.length() - 1 && !lastCharisReally) {
            word.isRealChar = false;
        }
        if (underlineHeight < word.height) {
            underlineHeight = word.height;
        }
		switch (nowWchar)
		{
		case 0x20:		 //L' '
// 			if (lineTopKong)
// 			{
// 				break;
// 			}
			word.width = nCHalfW;

			drawX += (nCHalfW + zijianju);
			if (lastWordIsEng && tianchong-- > 0)
			{
				drawX += 1;
				lastWordIsEng = false;
			}
			if (lineTopKong)
			{
				continue;;
			}
			break;			
		case 0x3000	:	//L'　'
// 			if (lineTopKong)
// 			{
// 				break;
// 			}
			word.width = nCFullW;

			drawX += (nCFullW + zijianju);
			if (lastWordIsEng && tianchong-- > 0)
			{
				drawX += 1;
				lastWordIsEng = false;
			}
			if (lineTopKong)
			{
				continue;;
			}
			break;			
		case 0x9 :		//L'\t'
			lineTopKong =false;
			word.width = nCFullW * m_nTabSize;

			drawX += ((nCFullW + zijianju) * m_nTabSize);
			if (lastWordIsEng && tianchong-- > 0)
			{
				drawX += 1;
				lastWordIsEng = false;
			}
			break;		
		case 0x0a:		//L'\n'
		case 0x0d:		//L'\r'
			goto exit;
			break;
		default:
			//word.y = tempDrawY;
			lineTopKong = false;
			m_PreWordinfo.y = tempDrawY;
			if (isEngChar(nowWchar))
			{
				lastWordIsEng = true;
				pEngFont->BuildCharCache(nowWchar);
				pNode = pEngFont->GetCharCacheNode(nowWchar);
				if (pNode == NULL)
				{
					drawX += nEFullW;
					continue;
				}
				RenderChar(image,pNode,drawX, tempDrawY,nCFullW,fontColor, word);
				word.width = pNode->charAdvance;
                if (underlineHeight < word.height) {
                    underlineHeight = word.height;
                }

				m_NowWordCharIndex = pEngFont->GetCharCacheNode(nowWchar)->charGlyphIndex;
				drawX += pEngFont->GetTwoCharKerning(m_PrevWordCharIndex,m_NowWordCharIndex);
                printf("render::drawline | charkerning:%d\n", pEngFont->GetTwoCharKerning(m_PrevWordCharIndex,m_NowWordCharIndex));
				drawX += pNode->w/*charAdvance*/;
				m_PrevWordCharIndex = m_NowWordCharIndex;
			}
			else
			{
				lastWordIsEng = false;
				int CLR = returnCharLeftRight(nowWchar);
				pChiFont->BuildCharCache(nowWchar);
				pNode = pChiFont->GetCharCacheNode(nowWchar);
				if (pNode == NULL)
				{
					drawX += nCFullW;
					continue;
				}
				else
				{
					wchar_t c = nowWchar;
					int BW(0),lk(0),rk(0);
					//规则:标点占字宽的2/3。左标点居左占剩余空白的1/4,居右占3/4。右标点跟左标点相反
					if (CLR < 3)
					{
						BW = CalcFloat(pNode->charAdvance,(float)2/3);
						// 右标点需要在画之前进行调整
						float times = (CLR == 1)?((float)1/4):((float)3/4);
						lk = CalcFloat(BW - pNode->w,times);
						rk = BW - pNode->w - lk;
						drawX += lk;
					}
					RenderChar(image,pNode,drawX, tempDrawY,nCFullW,fontColor,word);
					word.width = pNode->charAdvance;
                    if (underlineHeight < word.height) {
                        underlineHeight = word.height;
                    }
                    
					m_NowWordCharIndex = pNode->charGlyphIndex;
					drawX += pChiFont->GetTwoCharKerning(m_PrevWordCharIndex,m_NowWordCharIndex);
					drawX += (CLR == 4?pNode->charAdvance:pNode->w);
					drawX += rk;
					m_PrevWordCharIndex = m_NowWordCharIndex;
				}
				drawX += zijianju;
				if (tianchong-- > 0)
				{
					drawX += 1;
				}
			}

			
			break;
		}

		word.offset = i + beginPos;
		word.contentID = contentId;
		pwi->lines[pwi->lines.size() - 1].words.push_back(word);
	}
	
exit:

	//multimedia
	if (content->contentType & CT_Link)
	{
		mp.rightbottom_x = drawX;
		cpmm->push_back(mp);
		if (content->textStyle.UnderlineIsVaild && content->textStyle.isUnderline)
		{
			myPOINT s,e;
			s.x = beginX;
			s.y = tempDrawY + underlineHeight;
			e.x = drawX - zijianju;
			e.y = tempDrawY + underlineHeight;
			image.DrawLine(s,e,fontColor);
		}
	}
	else
	{
		if (content->isHaveStyle && content->textStyle.UnderlineIsVaild && content->textStyle.isUnderline)
		{
			myPOINT s,e;
			s.x = beginX;
			s.y = tempDrawY + underlineHeight;
			e.x = drawX - zijianju;
			e.y = tempDrawY + underlineHeight;
			image.DrawLine(s,e,fontColor);
		}
	}
	
	return 0;
}

int CRender::RenderChar( SZImageBit24 &Image,CharCacheNode* pNode,int drawX, int drawY ,int defaultAD,myRGBQUAD fontColor, wordinfo& wi)
{
	unsigned char *pSrc = /*isEng?*/pNode->data/*:pNode->glyph->bitmap.buffer*/;
	if (pSrc == NULL)
	{
		return -1;
	}

	if (defaultAD == 0)
	{
		defaultAD = pNode->charAdvance;
	}
	//drawX += pNode->x;
	myBYTE pixV;

	for(int x = 0 ; x <  pNode->w; x++)
	{
		for( int y =0 ; y < pNode->h; y++)
		{				
			pixV=pSrc[y * pNode->rowSize + x];
			Image.SetDesPixel(drawX + x,					
				m_OutDevH-(drawY+(pNode->ascender-pNode->y)+y),fontColor,pixV);
		}
	}
    
    
    wi.height = pNode->ascender-pNode->descender;
	return 0;
}

int CRender::DrawContentImage( SZImageBit24& pageImage,SZImageBase* image,Content* content ,MultimediaPage* cpmm,bool isRender)
{
	if (content == NULL || image == NULL) 
	{
		return -1;
	}

	int imageW = image->GetWidth();
	int imageH = image->GetHeight();

	int drawW = m_OutDevW - m_OffsetX - m_nMrginRight;
	int drawH = m_OutDevH - m_OffsetY - m_nMrginBottom;
	if (imageW > drawW || imageH > drawH)
	{
		return -2;
	}
	//暂时图片水平居中显示
	if (!isRender)
	{
		int offsetX = m_OffsetX + (drawW - imageW)/2;
		for (int j=0;j<imageH;j++)
		{	
			for (int i=0;i<imageW;i++)
			{
				if (m_iStop)
				{
					return 1;
				}
				pageImage.SetDesPixel(offsetX+i,m_OutDevH-1-(m_OffsetY+j),image->GetOriPixel(i,imageH-1-j),255);			
			}
		}
		cpmm->lefttop_x = offsetX;
		cpmm->lefttop_y = m_OffsetY;
		cpmm->rightbottom_x = offsetX + imageW;
		cpmm->rightbottom_y = m_OffsetY + imageH;
		
	}

	
	
	m_OffsetY += (imageH + m_nVertSpac);
	if (m_OffsetY > (m_OutDevH - m_nMrginBottom))
	{//该页绘制满了
		m_OffsetY = m_nMrginTop;
		m_OffsetX = m_nMrginLeft;
		return 0;

	}
	return 1;
}

void CRender::InitLineInfo()
{
	m_curlineinfo.Height = 0;
	m_curlineinfo.Width = 0;
	m_curlineinfo.RealwordNum = 0;
	m_curlineinfo.x = 0;
	m_curlineinfo.y = 0;
	m_curlineinfo.words.clear();
}

int CRender::DrawHighLight( SZImageBit24& image, SZFTFont* pChiFont,SZFTFont* pEngFont,vector<wordinfo>& vWordInfo,myRGBQUAD bgColor )
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
				DrawBackGround(image,linex,liney,width,height,bgColor);
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
		DrawBackGround(image,linex,liney,width,height,bgColor);
	}
	//写字
drawWord:
	int nFullW;
	nFullW =pChiFont->GetFullWidthAdvanceCH();
	for (i = 0;i < vWordInfo.size();i++)
	{
		wordinfo wi = vWordInfo[i];
		myWORD nowWchar = wi.WordUnicod;
		CharCacheNode* pNode =NULL;
		bool isEngFont;
		if (!isEngChar(nowWchar))
		{
			pChiFont->BuildCharCache(nowWchar);
			pNode = pChiFont->GetCharCacheNode(nowWchar);
		}
		else
		{
			pEngFont->BuildCharCache(nowWchar);
			pNode = pEngFont->GetCharCacheNode(nowWchar);
		}	 
		if (!pNode /*|| !pNode->data*/)
		{	 
			continue;			
		}else
		{
			RenderChar(image,pNode,wi.x,wi.y,nFullW,m_clrForeG,wi);
		}

	}
	return 0;
}


int CRender::DrawBackGround( SZImageBit24 &Image,int x,int y,int width,int height,myRGBQUAD bgColor)
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

int CRender::DrawImage( SZImageBit24& pageImage,SZImageBase* image,int x,int y )
{
	int imageW = image->GetWidth();
	int imageH = image->GetHeight();
		for (int j=0;j<imageH;j++)
		{	
			for (int i=0;i<imageW;i++)
			{
				pageImage.SetDesPixel(x+i,m_OutDevH-1-(y+j),image->GetOriPixel(i,imageH-1-j),255);			
			}
		}
	return 1;
}

void CRender::SetMrginLeft( int val )
{
	m_nMrginLeft = val;
	m_OffsetX = m_nMrginLeft;
}

void CRender::SetMrginTop( int val )
{
	m_nMrginTop = val;
	m_OffsetY = m_nMrginTop;
}
