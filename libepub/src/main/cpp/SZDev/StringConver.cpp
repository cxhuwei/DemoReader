#include "StdAfx.h"
#include "myfDev/GBK.h"
#include "myfDev/GB18030.h"
#include "StringConver.h"
#include "MAlgorithm.h"
//#include <string>
//#include <stdio.h>
/*#include <io.h>*/
#include "iconv.h"

CStringConver::CStringConver(void)
{

	if (sizeof(wchar_t) == 2)
	{
		m_unicodeString = "UCS-2-INTERNAL";
	}
	else if (sizeof(wchar_t) == 4)
	{
		m_unicodeString = "UCS-4-INTERNAL";
	}
}

CStringConver::~CStringConver(void)
{
}

void CStringConver::Utf8ToUnicodeBig(char* inText,int inlen, myWcharT* outText,int outlen)
{
#ifdef _LIBICONV
	iconv_t  converter = iconv_open(m_unicodeString,"utf-8");
	if (converter == iconv_t(-1))
	{
		return ;
	}
	char* input = inText;
	size_t st_inlen = inlen;
	size_t st_outlen = outlen;
	char* poutput = (char*)outText;
	size_t rc = iconv(converter, (char**)&input, &st_inlen, &poutput, &st_outlen);
	iconv_close(converter);

#else
	int length = strlen(inText);
	char* temp = (char*)outText;
	int iUnic=0;
	for (int i=0;i<length;)
	{
		int nByteNum = GetUtf8ByteNum(inText[i]);
		switch(nByteNum)
		{
		case 1:
			//���ֽ�
			temp[iUnic] = inText[i];
			temp[iUnic + 1] = 0; 
			break;
		case 2:
			//˫�ֽ�
			temp[iUnic] = (inText[i] << 6) + (inText[i + 1]  & 0x3F);
			temp[iUnic + 1] = (inText[i] >> 2) & 0x07;
// 			temp[iUnic] = inText[i];
// 			temp[iUnic + 1] = inText[i + 1];
			break;
		case 3:
			//���ֽ�
			temp[iUnic + 1] = ((inText[i] & 0x0F) << 4) | ((inText[i + 1] >> 2) & 0x0F);
			temp[iUnic] = ((inText[i + 1] & 0x03) << 6) + (inText[i + 2] & 0x3F);
			break;
		default:
			//������
			break;
		}
		i += nByteNum;
		myBYTE c = temp[iUnic];
		myBYTE c2 = temp[iUnic + 1];
		if (c == 0xa0 && c2 == 0x00)
		{
			temp[iUnic] = 0x20;
		}

		iUnic +=sizeof(myWcharT);
	}
#endif
}

void CStringConver::UnicodeToUtf8(myWcharT* inText,int inlen, char* outText,int outlen)
{
	iconv_t  converter = iconv_open("utf-8",m_unicodeString);
		if (converter == iconv_t(-1))
		{
			return ;
		}
		char* input = (char*)inText;
		size_t st_inlen = inlen;
		size_t st_outlen = outlen;
		char* poutput = outText;
		size_t rc = iconv(converter, (char**)&input, &st_inlen, &poutput, &st_outlen);
		iconv_close(converter);
}

int CStringConver::GetUtf8ByteNum(char firstCh)
{
	myBYTE b = firstCh;
	if (b < 0xc0)
	{
		return 1;
	}
	else if(b < 0xe0)
	{
		return 2;
	}
	else if (b < 0xf0)
	{
		return 3;
	}
	else 
		return 4;
}

void CStringConver::Utf8ToUnicodeSmall(char* inText, myWcharT* outText)
{
	int length = strlen(inText);
	char* temp = (char*)outText;
	int iUnic=0;
	for (int i=0;i<length;)
	{
		int nByteNum = GetUtf8ByteNum(inText[i]);
		switch(nByteNum)
		{
		case 1:
			//���ֽ�
			temp[iUnic] = 0;
			temp[iUnic + 1] = inText[i]; 
			break;
		case 2:
			//˫�ֽ�
			temp[iUnic + 1] = (inText[i] << 6) + (inText[i + 1]  & 0x3F);
			temp[iUnic] = (inText[i] >> 2) & 0x07;
			break;
		case 3:
			//���ֽ�
			temp[iUnic]		= ((inText[i] & 0x0F) << 4) | ((inText[i + 1] >> 2) & 0x0F);
			temp[iUnic + 1] = ((inText[i + 1] & 0x03) << 6) + (inText[i + 2] & 0x3F);
			break;
		default:
			//������
			break;
		}
		i += nByteNum;
		iUnic +=sizeof(myWcharT);
	}
}

void CStringConver::GbkToUnicodeBig(char* inText ,int inlen, myWcharT* outText,int outlen)
{
#ifdef _LIBICONV
	iconv_t  converter = iconv_open(m_unicodeString,"gbk");
	if (converter == iconv_t(-1))
	{
		return ;
	}
	char* input = inText;
	size_t st_inlen = inlen;
	size_t st_outlen = outlen;
	char* poutput = (char*)outText;
	size_t rc = iconv(converter, (char**)&input, &st_inlen, &poutput, &st_outlen);
	iconv_close(converter);

#else
	//1.step ����gbk������ļ�
	
	char* tempchar = (char*)outText;
	int length = strlen(inText);
	int iUnic=0;
	for (int i=0;i<length;)
	{
		myBYTE temp = inText[i];
		if (temp < 0x81)
		{//�������ַ�
			tempchar[iUnic] = inText[i++]; 
			tempchar[iUnic + 1] = 0;
		}
		else
		{
			myWcharT unico;
			GetUnicodeByGbkBig(inText + i,unico);
			char* tempunico = (char*)&unico;
			tempchar[iUnic]		= tempunico[0];
			tempchar[iUnic + 1] = tempunico[1];
			i += 2;
		}
		iUnic += sizeof(myWcharT);
	}
#endif // _LIBICONV
	
//#endif // _LIBICONV
}

int CStringConver::GetUnicodeByGbkBig(char* szGbk, myWcharT& szUnicode)
{
	unsigned short sGbk(0);
// 	char strGbk[5] = {0};
// 	char buffer[3] = {0};
/*	char* un;*/
 	myBYTE igbk_h = (myBYTE)szGbk[0];
// 	itoa(igbk_h,buffer,16);
// 	strcat(strGbk,buffer);

 	myBYTE igbk_l = (myBYTE)szGbk[1];
// 	itoa(igbk_l,buffer,16);
// 	strcat(strGbk,buffer);

	sGbk = sGbk | igbk_h;
	sGbk = sGbk << 8;
	sGbk = sGbk | igbk_l;

	int offer=0;
	for (;offer < 21887;offer++)
	{
// 		char* gbk = G_GBK[offer];
// 		if(strcmpi(strGbk,gbk) == 0)
// 		{
// 			break;
// 		}		
		if (sGbk == G_GBK[offer])
		{
			break;
		}
	}

	if (offer < 21887)
	{
		szUnicode = G_UNICODE[offer];
		//un = G_UNICODE[offer];
// 		char* p;
// 		long lunicode = strtol(un,&p,16);
// 		szUnicode = lunicode;
	}

	return 0;
}

int CStringConver::GetUnicodeByGb18030Big(char* szGb18030,myWcharT& szUnicode)
{
	myBYTE igb18030_h = (myBYTE)szGb18030[0];
	int temp1 = igb18030_h - 129;
	myBYTE igb18030_l = (myBYTE)szGb18030[1];
	int temp2 = igb18030_l - 64;
	int offer = temp1 * 190 + temp2;
	
	if (offer < 23940)
	{
		szUnicode = G_GB18030[offer];	
	}
	else
		szUnicode = 0;
	return 0;
}

void CStringConver::Gb18030ToUnicodeBig(char* inText,int inlen,myWcharT* outText,int outlen)
{
#ifdef _LIBICONV
	iconv_t  converter = iconv_open(m_unicodeString,"GB18030");
	if (converter == iconv_t(-1))
	{
		return ;
	}
	char* input = inText;
	size_t st_inlen = inlen;
	size_t st_outlen = outlen;
	char* poutput = (char*)outText;
	size_t rc = iconv(converter, (char**)&input, &st_inlen, &poutput, &st_outlen);
	iconv_close(converter);

#else
	char* tempchar = (char*)outText;
	int length = strlen(inText);
	int iUnic=0;
	for (int i=0;i<length;)
	{
		myBYTE temp = inText[i];
		if (temp < 0x81)
		{//�������ַ�
			tempchar[iUnic] = inText[i++]; 
			tempchar[iUnic + 1] = 0;
		}
		else
		{
			myWcharT unico;
			GetUnicodeByGb18030Big(inText + i,unico);
			char* tempunico = (char*)&unico;
			tempchar[iUnic] = tempunico[0];
			tempchar[iUnic + 1] = tempunico[1];
			i += 2;
		}
		iUnic += sizeof(myWcharT);
	}
#endif
}

int CStringConver::GetUnicodeByGbkSmall( char* szGbk, myWcharT& szUnicode )
{
	unsigned short sGbk(0);
	myBYTE igbk_h = (myBYTE)szGbk[1];
	myBYTE igbk_l = (myBYTE)szGbk[0];

	sGbk = sGbk | igbk_h;
	sGbk = sGbk << 8;
	sGbk = sGbk | igbk_l;

	int offer=0;
	for (;offer < 21887;offer++)
	{
	
		if (sGbk == G_GBK[offer])
		{
			break;
		}
	}

	if (offer < 21887)
	{
		szUnicode = G_UNICODE[offer];
		//�ߵͻ���
		myBYTE temp;
		temp = szUnicode >> 8;
		szUnicode = szUnicode << 8;
		szUnicode = szUnicode | temp;
	}

	return 0;

}

void CStringConver::GbkToUnicodeSmall( char* inText , myWcharT* outText )
{
	char* tempchar = (char*)outText;
	int length = strlen(inText);
	int iUnic=0;
	for (int i=0;i<length;)
	{
		myBYTE temp = inText[i];
		if (temp < 0x81)
		{//�������ַ�
			tempchar[iUnic] = 0; 
			tempchar[iUnic + 1] = inText[i++];
		}
		else
		{
			myWcharT unico;
			GetUnicodeByGbkSmall(inText + i,unico);
			char* tempunico = (char*)&unico;
			tempchar[iUnic]		= tempunico[0];
			tempchar[iUnic + 1] = tempunico[1];
			i += 2;
		}
		iUnic += sizeof(myWcharT);
	}
}

int CStringConver::GetUnicodeByGb18030Small( char* szGb18030,myWcharT& szUnicode )
{
	myBYTE igb18030_h = (myBYTE)szGb18030[1];
	int temp1 = igb18030_h - 129;
	myBYTE igb18030_l = (myBYTE)szGb18030[0];
	int temp2 = igb18030_l - 64;
	int offer = temp1 * 190 + temp2;

	if (offer < 23940)
	{
		szUnicode = G_GB18030[offer];	
		//�ߵͻ���
		myBYTE temp;
		temp = szUnicode >> 8;
		szUnicode = szUnicode << 8;
		szUnicode = szUnicode | temp;
	}
	else
		szUnicode = 0;
	return 0;
}

void CStringConver::Gb18030ToUnicodeSmall( char* inText,myWcharT* outText )
{
	char* tempchar = (char*)outText;
	int length = strlen(inText);
	int iUnic=0;
	for (int i=0;i<length;)
	{
		myBYTE temp = inText[i];
		if (temp < 0x81)
		{//�������ַ�
			tempchar[iUnic] = 0; 
			tempchar[iUnic + 1] = inText[i++];
		}
		else
		{
			myWcharT unico;
			GetUnicodeByGb18030Small(inText + i,unico);
			char* tempunico = (char*)&unico;
			tempchar[iUnic] = tempunico[0];
			tempchar[iUnic + 1] = tempunico[1];
			i += 2;
		}
		iUnic += sizeof(myWcharT);
	}
}

void CStringConver::Lin_Utf8ToU16( wchar_t utf8,wchar_t& u16 )
{
	int length = sizeof(utf8);
	char* inText = (char*)&utf8;
	char* temp = (char*)&u16;
	memset(temp,0,4);
	int iUnic=0;
	int nByteNum = GetUtf8ByteNum(inText[0]);
	switch(nByteNum)
	{
	case 1:
		//���ֽ�
		temp[iUnic] = inText[0];
		temp[iUnic + 1] = 0; 
		break;
	case 2:
		//˫�ֽ�
		temp[iUnic] = (inText[0] << 6) + (inText[1]  & 0x3F);
		temp[iUnic + 1] = (inText[0] >> 2) & 0x07;
		// 			temp[iUnic] = inText[i];
		// 			temp[iUnic + 1] = inText[i + 1];
		break;
	case 3:
		//���ֽ�
		temp[iUnic + 1] = ((inText[0] & 0x0F) << 4) | ((inText[ 1] >> 2) & 0x0F);
		temp[iUnic] = ((inText[1] & 0x03) << 6) + (inText[2] & 0x3F);
		break;
	default:
		//������
		break;
	}
	myBYTE c = temp[iUnic];
	myBYTE c2 = temp[iUnic + 1];
	if (c == 0xa0 && c2 == 0x00)
	{
		temp[iUnic] = 0x20;
	}
}

void CStringConver::Lin_U16ToUtf8( wchar_t u16,wchar_t& utf8 )
{
	char* inText = (char*)&u16;
	char* temp = (char*)&utf8;
	memset(temp,0,4);
 	if (u16 <= 0x007f)
	{
		temp[0] = inText[0] & 0x7F;
	}
	else if (u16<0x0800)
	{
		temp[0] = u16>>6 & 0x1F | 0xC0;
		temp[1] = u16>>0 & 0x3F | 0x80;
	}
	else if (u16<0x010000) 
	{
		temp[0] = u16>>12 & 0x0F | 0xE0;
		temp[1] = u16>>6 & 0x3F | 0x80;
		temp[2] = u16>>0 & 0x3F | 0x80;
	}
	else if (u16<0x110000) 
	{
		temp[0] = u16>>18 & 0x07 | 0xF0;
		temp[1] = u16>>12 & 0x3F | 0x80;
		temp[2] = u16>>6 & 0x3F | 0x80;
		temp[3] = u16>>0 & 0x3F | 0x80;
	}
}

int CStringConver::GetGbkByUnicodeBig( myWcharT szUnicode,char* szGbk )
{
	unsigned short sGbk(0);

	int offer=0;
	for (;offer < 21887;offer++)
	{
		if (szUnicode == G_UNICODE[offer])
		{
			break;
		}
	}

	if (offer < 21887)
	{
		sGbk = G_GBK[offer];
		szGbk[1] = sGbk & 0xff;
		sGbk = sGbk >> 8;
		szGbk[0] = sGbk;
	}

	return 0;
}

void CStringConver::UnicodeBigToGbk( myWcharT* inText,int inlen,char* outText,int outlen )
{
#ifdef _LIBICONV
	iconv_t  converter = iconv_open("gbk",m_unicodeString);
	if (converter == iconv_t(-1))
	{
		return ;
	}
	char* input = (char*)inText;
	size_t st_inlen = inlen;
	size_t st_outlen = outlen;
	char* poutput = outText;
	size_t rc = iconv(converter, (char**)&input, &st_inlen, &poutput, &st_outlen);
	iconv_close(converter);

#else
	int len = myWcslen(inText);
	int iAnsi=0;
	for (int i=0;i < len; i++)
	{
		myWcharT wchar = inText[i];
		short temp = wchar & 0xff;
		if (wchar >= 0x4E00 && wchar <=  0x9FA5 || wchar >= 0x3400 && wchar <= 0x4dbf || wchar >= 0x4DC0 && wchar <= 0x4DFF 
			|| wchar >= 0x2E80 && wchar <= 0x2EFF || wchar >= 0x3000 && wchar <= 0x303F)
		{
			char szansi[2];
			memset(szansi,0,2);
			GetGbkByUnicodeBig(wchar,szansi);
			outText[iAnsi++] = szansi[0];
			outText[iAnsi++] = szansi[1];
		}
		else
		{
			outText[iAnsi++] = wchar >> 8;
		}
	}
#endif
}

void CStringConver::UnicodeSmallToUnicodeBig( char* inText,int inlen,myWcharT* outText,int outlen )
{
#ifdef _LIBICONV
	char* unicodeb;
	if (sizeof(wchar_t) == 2)
	{
		unicodeb = "UCS-2LE";
	}
	else if (sizeof(wchar_t) == 4)
	{
		unicodeb = "UCS-4LE";
	}
	iconv_t  converter = iconv_open(unicodeb,"UCS-2BE");
	if (converter == iconv_t(-1))
	{
		return ;
	}
	char* input = (char*)inText;
	size_t st_inlen = inlen;
	size_t st_outlen = outlen;
	char* poutput = (char*)outText;
	size_t rc = iconv(converter, (char**)&input, &st_inlen, &poutput, &st_outlen);
	iconv_close(converter);

#endif
}
