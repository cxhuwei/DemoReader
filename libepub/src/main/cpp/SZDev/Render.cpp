#include "stdafx.h"
#include "Render.h"
#include "MAlgorithm.h"

#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "PDGPARSER"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG ,__VA_ARGS__)
#endif


const float g_Paragraph = 2.5;//�μ�����м��ı���
CRender::CRender(void)
{
	m_PrevWordCharIndex = 0;
	m_NowWordCharIndex = 0;
	m_nMrginLeft = 50;
	m_nMrginRight = 50;
	m_nMrginTop = 50;
	m_nMrginBottom = 50;
	m_nTabSize = 2;//������2���ֵĿ�ȣ�
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
	beginPos ����content�У���beginPos��ʼ����
	endpos: ���ظ��ϲ㣬��content�У����Ƶ����һ��λ��
	endpos :-2 ��content��������
	endpos :-1 ��content��û�л��ƣ�offsety�Ѿ�������Χ�ˡ�

*/
int CRender::DrawContentText( SZImageBit24& image,SZFTFont* pChiFont,SZFTFont* pEngFont,int contentId,Content* content,int beginPos,int& endPos ,pageWordinfo* pwi,Vt_curPageMultiMedia* cpmm,bool isRender)
{
	if (pChiFont == NULL || pEngFont == NULL || content == NULL)
	{
		return -1;
	}
	int nCFullW=pChiFont->GetFullWidthAdvanceCH();//�������ֿ��
	int nCHalfW=pChiFont->GetHalfWidthAdvanceCH();//���İ��ֿ��
	int nEFullW=pEngFont->GetFullWidthAdvance();//Ӣ�����ֿ��
	int nEHalfW=pEngFont->GetHalfWidthAdvance();//Ӣ�����ֿ��

	//��content�е��ı�ת�ɵ�һ���ַ�����
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
		{//�������ǰ�汾��û����������ô�ֶ���������
			//strText.insert(0,L"\t");//��ʼ����һ���Ʊ��
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

	//��ʼ����

	//��ʽ
	if (content->isHaveStyle)
	{
		pChiFont->SetIsBold(content->textStyle.Bold);
		pChiFont->SetIsItalic(content->textStyle.isItalic);
		pEngFont->SetIsBold(content->textStyle.Bold);
		pEngFont->SetIsItalic(content->textStyle.isItalic);
	}

	int drawX = m_OffsetX;//m_offsetX��¼���ϴλ��Ƶĺ�����
	//int drawY = m_OffsetY;//m_offsetY��¼���ϴλ��Ƶ�������

	int textLength,blankLenght;
	int tempPrevWordIndex = m_PrevWordCharIndex;
	int deleteCount(0);//������Ҫɾ����word����
	if ((m_OffsetY + nCFullW)> (m_OutDevH - m_nMrginBottom))
	{
		endPos--;
		m_OffsetX = m_nMrginLeft;
		m_OffsetY = m_nMrginTop;
		return 0;
	}
	//��һ����ͳ��һ���ܷŵ��µ�������
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
		{//��������
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
			//�ѻ��������ƶ�����һ��
			drawX = m_OffsetX = m_nMrginLeft;
			m_OffsetY += (m_curLineMaxHeight/*nEFullW*/ + m_nParagraphSpac*nCFullW/*m_nVertSpac * g_Paragraph*/);//����֮����2���о����μ��
			m_curLineMaxHeight = 0;
			if (m_OffsetY > (m_OutDevH - m_nMrginBottom - nEFullW))
			{//��ҳ��������
				m_OffsetY = m_nMrginTop;
				m_OffsetX = m_nMrginLeft;
				if (endPos == strText.length() - 1)
				{//���һ��
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
			else if (nowWchar == 0x3000)//L'��'
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
						//����:���ռ�ֿ��2/3���������ռʣ��հ׵�1/4,����ռ3/4���ұ��������෴
						if (CLR < 3)
						{
							BW = CalcFloat(pNode->charAdvance,(float)2/3);
							// �ұ����Ҫ�ڻ�֮ǰ���е���
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
// 						{//��߱��
// 							drawX += pNode->x;//��ʵ�ʱ�������߿�������ӵ����֮����������һ���ֵľ���
// 						}
							
						}
					m_PrevWordCharIndex = m_NowWordCharIndex;
				}
			}

			if (drawX > (m_OutDevW - m_nMrginRight))
			{//�л��Ƶ�ĩβ�ˣ���Ҫ������
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
				//��Ա��������ֵ���
				
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
				
				//�ѻ��������ƶ�����һ��
				drawX = m_OffsetX = m_nMrginLeft;
				m_OffsetY += (m_curLineMaxHeight/*nEFullW*/ + m_nVertSpac*nCFullW);
				m_curLineMaxHeight = 0;
				if (m_OffsetY > (m_OutDevH - m_nMrginBottom - nEFullW))
				{//��ҳ��������
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
		//��ǰcontent��ʣ���
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
	endPos = -2;//��Ǹ�contentȫ��������
	return 1;
}


void CRender::Punctuation( std::wstring strFull,int index,std::wstring& strLine,int& deleteCount)
{
	//index  �Ѿ������߽���Ǹ��ַ�������
	myWORD nowWchar;
	myWORD beforWchar;
	wstring temp = strLine;
	deleteCount = 0;//��󳬹��߽���Ǹ��ַ���û�з��뵽strLine��
	if (index <= 0 || index >= strFull.size())
	{
		return;
	}
	nowWchar = strFull[index];

	//������Χ�����һ����Ӣ�� ����������ǰ��һ����ĸҲɾ������һ�����
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
	{//��ĩ���;��ĩ��㲻�ܳ��������ף�������Ҫ��ǰһ���ַ�����ǰ�ַ�һ��ŵ���һ��
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
	
	//��⴮���һ���ַ��ǲ��Ǿ��ױ��
	
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
	drawX:���Ƹ��е���ʼ������
	drawY:���Ƹ��е���ʼ������
	strX:���л��Ƶ��Ҷ˺�����
	wordCountInstr:�ַ������ֵĸ���
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

	int nCFullW=pChiFont->GetFullWidthAdvanceCH();//�������ֿ��
	int nCHalfW=pChiFont->GetHalfWidthAdvanceCH();//���İ��ֿ��
	int nEFullW=pEngFont->GetFullWidthAdvance();//Ӣ�����ֿ��
	int nEHalfW=pEngFont->GetHalfWidthAdvance();//Ӣ�����ֿ��

	//���˶��룬Ԥ�Ű�
	int beginX = drawX;
	int countLength = m_OffsetX;
	int zijianju = m_nWordSpac;
	int tianchong(0);//ƽ����̯��ǰn���ּ��һ������
	bool lastWordIsEng(false);
	int lineWordCount = 0;//��¼һ����������ָ���(�������ո��Ʊ�������з�,Ӣ�ĵ�����һ��)
	int tempPrevWordIndex = m_PrevWordCharIndex;
	bool lineTopKong = true;//���׵Ŀո�ȥ��
	if (!(str[str.length() - 1] == L'\n' || !isNeedJustify))//1��str�л��з�����Ҫ���˶��룻2.������ǲ���Ҫ����
	{
		//�������в���Ҫ���˶���
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
			case 0x3000	:	//L'��'
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
						//����:���ռ�ֿ��2/3���������ռʣ��հ׵�1/4,����ռ3/4���ұ��������෴
						if (CLR < 3)
						{
							BW = CalcFloat(pNode->charAdvance,(float)2/3);
							// �ұ����Ҫ�ڻ�֮ǰ���е���
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
// 						{//��߱��
// 							countLength += pNode->x;//��ʵ�ʱ�������߿�������ӵ����֮����������һ���ֵ�?��?
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

	myRGBQUAD fontColor = m_clrForeG;//������ɫ
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
	{//�ϱ� �����������ƶ�����ָ�
		tempDrawY -= nCHalfW;
	}
	if (content->contentType & CT_Link)
	{//���� ���û�ж��������Լ���ɫ����ʹ��Ĭ����ɫ
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
	//��ʼ��

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
		case 0x3000	:	//L'��'
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
					//����:���ռ�ֿ��2/3���������ռʣ��հ׵�1/4,����ռ3/4���ұ��������෴
					if (CLR < 3)
					{
						BW = CalcFloat(pNode->charAdvance,(float)2/3);
						// �ұ����Ҫ�ڻ�֮ǰ���е���
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
	//��ʱͼƬˮƽ������ʾ
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
	{//��ҳ��������
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
	//д��
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
			Image.SetDesPixel(x + i,ImgH - m_nVertSpac/2 - (y + j),bgColor,255);//�˴���3��΢��������
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
