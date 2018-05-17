#include "StdAfx.h"
#include "MAlgorithm.h"
#include <cstring>
#include <stdio.h>
#include <ctype.h>

#ifndef _WINDOWS
#include <unistd.h>
#endif
int m_atoi(const char* str,int radix)
{
	int num(0);
	int length = strlen(str);
	for(int i=0;i<length;i++)
	{
		if(str[i]>='0'&&str[i]<='9')
		{
			num += (int(str[i])-48) * pow((double)radix,length - i - 1);
		}
		else if(str[i]>='a'&&str[i]<='f')
		{
			num += (int(str[i])-87) * pow((double)radix,length - i - 1);
		}
		else if(str[i]>='A'&&str[i]<='F')
		{
			num += (int(str[i])-55)* pow((double)radix,length - i - 1);
		}
		else 
			return -1;

	}
	return num;
}
char* m_ToLower(const char* szstr)
{
	char* orig = (char*)szstr; 
	char* p = orig;
	//   process   the   string 
	for ( ; *p != 0x00; p++) 
		*p = tolower(*p); 
	return orig; 
}
bool isEngChar(myWORD wchar)
{
    
    if (wchar == 0x0101 || wchar == 0x00e1 || wchar == 0x01ce || wchar == 0x00e0
        || wchar == 0x014d || wchar == 0x00f3 || wchar == 0x01d2 || wchar == 0x00f2
        || wchar == 0x0113 || wchar == 0x00e9 || wchar == 0x011b || wchar == 0x00e8
        || wchar == 0x012b || wchar == 0x00ed || wchar == 0x01d0 || wchar == 0x00ec
        || wchar == 0x016b || wchar == 0x00fa || wchar == 0x01d4 || wchar == 0x00f9
        || wchar == 0x01d6 || wchar == 0x01d8 || wchar == 0x01da || wchar == 0x01dc || wchar == 0x00fc) {
        return true;
    }
	if (wchar >= 97 && wchar <= 122 || wchar >= 65 && wchar <= 90 || wchar == 45/* - */ || wchar == 37/* % */)
	{
		return true;
	}
	else
		return false;
}

// 返回wchar字的类型
//0:未知符号
// 1:句末标点
// 2:句首标点
// 3:英文
// 4:中文
int returnCharType(myWORD wchar)
{
	if (wchar == 0)
	{
		return 0;
	}
	if((wchar==0x002c)||(wchar==0xff0c)||(wchar==0x002e)||(wchar==0x3002)||(wchar==0x0021)
		||(wchar==0xff01)||(wchar==0x003f)||(wchar==0xff1f)||(wchar==0x3001)||(wchar==0x003a)
		||(wchar==0xff1a)||(wchar==0x003b)||(wchar==0xff1b)||(wchar==0x0027)||(wchar==0x2019)
		||(wchar==0x0022)||(wchar==0x201d)||(wchar==0x0029)||(wchar==0xff09)||(wchar==0x005d)
		||(wchar==0x3011)||(wchar==0x007d)||(wchar==0xff5d)||(wchar==0x003e)||(wchar==0x300b)
		||(wchar==0x300d)||(wchar==0x300f)||(wchar==0x3009)||(wchar==0xff3d))
	{
		return 1;
	}
	else if ((wchar==0x2018)||(wchar==0x201c)||(wchar==0x0028)||(wchar==0xff08)||(wchar==0x005b)
		||(wchar==0x3010)||(wchar==0x007b)||(wchar==0xff5b)||(wchar==0x003c)||(wchar==0x300a)
			||(wchar==0x300c)||(wchar==0x300e)||(wchar==0x3008)||(wchar==0xff3b))
	{
		return 2;
	}
	else if (isEngChar(wchar))
	{
		return 3;
	}
	else 
		return 4;
}
int returnCharWidth(myWORD wchar, int nFullw, int nHalfw,SZFTFont *pEnglishFont,SZFTFont *pChFont)
{
	if (isEngChar(wchar))
	{
		pEnglishFont->BuildCharCache(wchar);
		CharCacheNode* temp = pEnglishFont->GetCharCacheNode(wchar);
		return temp->w;
	}	
	if (((wchar >> 8) & 0xff) == 0)//半角
	{
		return nHalfw;	
	}
	else 
	{
		int re = returnCharLeftRight(wchar);
		if ( re == 1 || re == 2)
		{
			pChFont->BuildCharCache(wchar);
			CharCacheNode* temp = pChFont->GetCharCacheNode(wchar);
			if (temp)
			{
				return temp->x+temp->w;
				delete temp;
			}
		}
		return nFullw;
	}
}
string& m_ToLower(string& str)
{
	transform(str.begin(),str.end(),str.begin(),::tolower);
	return str;
}
int m_Trim(char* szstr)
{
	int i,j;
	for (j = strlen(szstr) - 1;j >= 0;j--)
	{
		if (szstr[j] != ' ')
		{
			break;
		}
	}
	for (i = 0;i<strlen(szstr)&&j>=0;i++)
	{
		if (szstr[i] != ' ')
		{
			break;
		}
	}
	if (j == -1)
	{
		szstr[0] = '\0';
		return 0;
	}
	int x;
	for (x = 0;x<j-i+1;x++)
	{
		szstr[0 + x] = szstr[i + x];
	}
	szstr[j-i+1] = '\0';
	return 0;
}
string& m_TrimString(string& str)
{
	const string t = " ";
	str.erase(str.find_last_not_of(t) + 1);
	str.erase(0,str.find_first_not_of(t));
	char c = 0x0a;
	str.erase(str.find_last_not_of(c) + 1);
	str.erase(0,str.find_first_not_of(c));
	c = 0x0d;
	str.erase(str.find_last_not_of(c) + 1);
	str.erase(0,str.find_first_not_of(c));
	return str;
}

int m_Trim( myWcharT* szstr )
{
	int i,j;
	for (j = wcslen(szstr) - 1;j >= 0;j--)
	{
		if (szstr[j] != L' ')
		{
			break;
		}
	}
	for (i = 0;i<wcslen(szstr)&&j>=0;i++)
	{
		if (szstr[i] != L' ')
		{
			break;
		}
	}
	if (j == -1)
	{
		szstr[0] = L'\0';
		return 0;
	}
	int x;
	for (x = 0;x<j-i+1;x++)
	{
		szstr[0 + x] = szstr[i + x];
	}
	szstr[j-i+1] = L'\0';
	return 0;
}



int myWcslen(myWcharT* str)
{
	if (str == NULL)
		return 0;
#if defined (_WIN32) || defined(WIN32)
	return wcslen(str);
#else /* Linux */
	int len(0);
	while (str[len] != 0x0000)
	{
		len++;
	}
	return len;
#endif
	
}
int myWcscmp(myWcharT* string1,myWcharT* string2)
{
#if defined (_WIN32) || defined(WIN32)
	return wcscmp(string1,string2);
#else /* Linux */
	int i;
	for(i=0;string1[i]==string2[i];i++)
		if(string1[i]=='\0')
			return 0;
	return string1[i]-string2[i];
#endif
}

EncodingType CheckFileEncodingType(const char* filename)
{
	FILE* _file=fopen(filename,"rb");
	if (NULL==_file)
	{
		return OPENFAIL;
	}
	
	myBYTE* buf=new myBYTE[4];
	size_t nRead=fread((void*)buf,sizeof(myBYTE),4,_file);
	fclose(_file);
	
	if(nRead<2)
	{ 
		delete []buf;
		return READERR;
	}
	
	myBYTE utf32_le[]={0xFF,0xFE,0x00,0x00};
	if(memcmp(buf,&utf32_le,4)==0)
	{
		delete []buf;
		return UTF32_LE;
	}
	
	myBYTE utf32_be[]={0x00,0x00,0xFE,0xFF};
	if(memcmp(buf,&utf32_be,4)==0)
	{
		delete []buf;
		return UTF32_BE;
	}
	
	myBYTE utf_8[]={0xEF,0xBB,0xBF};
	if(memcmp(buf,&utf_8,3)==0)
	{
		delete []buf;
		return UTF_8;
	}
	
	myBYTE utf16_le[]={0xFF,0xFE};
	if(memcmp(buf,&utf16_le,2)==0)
	{
		delete []buf;
		return UTF16_LE;
	}
	
	myBYTE utf16_be[]={0xFE,0xFF};
	if(memcmp(buf,&utf16_be,2)==0)
	{
		delete []buf;
		return UTF16_BE;
	}
	if (buf[0] >= 0xe0 && buf[0] <0xf0 && buf[1] >=0x80 && buf[1] < 0xc0 && buf[2] >= 0x80 && buf[2] < 0xc0)
	{
		delete[] buf;
		return UTF_8;
	}
	delete []buf;
	return ANSI;
}	

int myStrCaseCmp( const char* str1, const char* str2 )
{
#ifdef _WINDOWS
	return stricmp(str1,str2);
#else
	return strcasecmp(str1,str2);
#endif // _WINDOWS
}

void mySleep( myDWORD dwMilliseconds )
{
#ifdef _WINDOWS
	Sleep(dwMilliseconds);
#else
	usleep(dwMilliseconds);
#endif // _WINDOWS
}


/*1 - 左半边
  2 - 右半边
  3 - 全宽
  4	- …（特殊处理）
*/
int returnCharLeftRight( myWORD wchar )
{
	if (wchar==0x300a /*《*/|| wchar == 0x300c/*「*/ || wchar==0x300e /*『*/ 
		|| wchar == 0x201c /*“*/ || wchar == 0x2018 /*‘*/|| wchar == 0xff08/*（*/ || wchar==0x3010/*【*/ || wchar==0xff5b /*｛*/
		||  wchar==0x3008/*〈*/ || wchar==0xff3b /*［*/|| wchar == 0x3014/*〔*/)
	{
		return 2;
	}
	else if (wchar == 0x300b /*》*/ || wchar==0x300d/*」*/ || wchar==0x300f/*』*/
		|| wchar == 0x201d /*”*/ ||  wchar == 0x2019/* ’*/ || wchar == 0xff09/*）*/ ||wchar == 0x3011/*】*/ || wchar == 0xff5d/*｝*/	
		|| wchar == 0x3009/*〉*/ || wchar == 0xff3d/*］*/ ||  wchar == 0x3015/*〕*/
		|| wchar == 0xff0c/*，*/ || wchar == 0x3002/*。*/ || wchar == 0xff01/*！*/ || wchar == 0x3001/*、*/ || wchar == 0xff1f /*？*/
		|| wchar == 0xff1b/*；*/ ||wchar == 0xff1a/*：*/)
	{
		return 1;
	}
	else if (wchar == 0x2026)
	{
		return 4;
	}
	else
		return 3;
}

int SpcaceNumber( myWcharT* szstr )
{
	int count(0);
	for (int j = wcslen(szstr) - 1;j >= 0;j--)
	{
		if (szstr[j] == L' ')
		{
			count++;
		}
	}
	return count;
}

int GetParentDirNumber( string& strHref )
{
	int number(0);
	string::size_type pos = strHref.find("../");
	while (pos != string::npos)
	{
		number++;
		strHref.erase(pos,3);
		pos = strHref.find("../");
	}
	return number;
}

string& MergerDir( string& dir, string strHref )
{
	if (dir.size() == 0)
	{
		dir = strHref;
		return dir;
	}
	int number = GetParentDirNumber(strHref);
	ProcessDirByParentDirNumber(dir,number);
	if (dir != "")
	{
		if (dir.at(dir.size() - 1) != '/')
		{
			dir.append("/");
		}
	}
	
	dir += strHref;
	return dir;
}

string& ProcessDirByParentDirNumber( string& dir,int number )
{
	if (dir.size() == 0)
	{
		return dir;
	}
	if (dir.at(dir.size()-1) == '/')
	{
		dir.erase(dir.size()-1);
	}
	while (number--)
	{
		string::size_type pos = dir.find('/');
		if (pos != string::npos)
		{
			dir.erase(pos,dir.size()-pos);
		}
		else
		{
			dir = "";
		}
	}
	return dir;
}

int CalcFloat( int value, float times )
{
	float fValue = (float)value * times;
	int iValue = int(fValue + 0.5);
	return iValue;
}
