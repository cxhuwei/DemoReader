#if !defined(STRINGCONVER_H)
#define STRINGCONVER_H


#include "HtmlParser.h"
class CStringConver
{
public:
	CStringConver(void);
	~CStringConver(void);
	void Utf8ToUnicodeBig(char* inText,int inlen, myWcharT* outText,int outlen);
	void UnicodeToUtf8(myWcharT* inText,int inlen, char* outText,int outlen);
	void Utf8ToUnicodeSmall(char* inText, myWcharT* outText);
	void GbkToUnicodeBig(char* inText ,int inlen, myWcharT* outText,int outlen);
	void GbkToUnicodeSmall(char* inText , myWcharT* outText);
	int GetUnicodeByGbkBig(char* szGbk, myWcharT& szUnicode);
	int GetUnicodeByGbkSmall(char* szGbk, myWcharT& szUnicode);
	int GetUnicodeByGb18030Big(char* szGb18030,myWcharT& szUnicode);
	int GetUnicodeByGb18030Small(char* szGb18030,myWcharT& szUnicode);
	void Gb18030ToUnicodeBig(char* inText,int inlen,myWcharT* outText,int outlen);
	void Gb18030ToUnicodeSmall(char* inText,myWcharT* outText);
	static void Lin_Utf8ToU16(wchar_t utf8,wchar_t& u16);
	static void Lin_U16ToUtf8(wchar_t u16,wchar_t& utf8);

	int GetGbkByUnicodeBig(myWcharT szUnicode,char* szGbk);
	void UnicodeBigToGbk(myWcharT* inText,int inlen,char* outText,int outlen);

	void UnicodeSmallToUnicodeBig(char* inText,int inlen,myWcharT* outText,int outlen);
 
private:
	static int GetUtf8ByteNum(char firstCh);	

	char	*	m_unicodeString;
};
#endif // !defined(STRINGCONVER_H)
