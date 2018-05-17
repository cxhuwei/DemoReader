#ifndef _MALGORITHM_H_
#define _MALGORITHM_H_
#include "myfDev/MYFDefine.h"
#include <algorithm>
#include <math.h>
#include "../SZDev/SZFTFont.h"

using namespace std;
//char*
int m_atoi(const char* str,int radix);
char* m_ToLower(const char* szstr);
int m_Trim(char* szstr);
bool isEngChar(myWORD wchar);
int returnCharType(myWORD wchar);
int returnCharWidth(myWORD wchar, int nFullw, int nHalfw,SZFTFont *pEnglishFont,SZFTFont *pChFont);
int returnCharLeftRight(myWORD wchar);
int myStrCaseCmp(const char* str1, const char* str2);
//string
string& m_ToLower(string& str);
string& m_TrimString(string& str);

//float
int CalcFloat(int value, float times);
//myWcharT
int myWcscmp(myWcharT* string1,myWcharT* string2);
int myWcslen(myWcharT* str);
int m_Trim(myWcharT* szstr);

int SpcaceNumber(myWcharT* szstr);

//system
void mySleep(myDWORD dwMilliseconds);

//encode
enum EncodingType {OPENFAIL=-1,READERR=0,ANSI=1,UTF16_LE,UTF16_BE,UTF32_LE,UTF32_BE,UTF_8};
EncodingType CheckFileEncodingType(const char* filename);

//href
int GetParentDirNumber(string& strHref);//得到href中../个数
string& ProcessDirByParentDirNumber(string& dir,int number);
string& MergerDir(string& dir, string strHref);
#endif
