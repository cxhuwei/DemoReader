#ifndef _STYLESHEETPARSER_H_
#define _STYLESHEETPARSER_H_

#include "myfDev/MYFDefine.h"


typedef map<std::string,std::string> Attributes;

typedef struct _Selector
{
	std::string selectorName;
	myBYTE selectorType;
	Attributes attrs;
}Selector;

class CStyleSheetParser
{
public:
	CStyleSheetParser(void);
	~CStyleSheetParser(void);
	void PutData(myBYTE* data,int length);
	int Parser();
	void Reset();
	
	map<string,StyleSheet> m_MCSS;
	//std::vector<Selector> m_vSelector;
private:
	myBYTE* m_cssData;
	int		m_cssLength;
	
private://Ã¿Ò»¸ö¿é
	std::string m_TagName;
// 	string m_ClassName;
// 	string m_IDName;
private:
	static const myBYTE START_DOCUMENT;
	static const myBYTE ANNOTATE;
	static const myBYTE START_TAG;
	static const myBYTE END_TAG;

	static const myBYTE TAG;
	static const myBYTE CLASS;// .
	static const myBYTE ID;//    #
	static const myBYTE ATTR;//  []

	static const myBYTE ATTRIBUTE_NAME;
	static const myBYTE ATTRIBUTE_VAULE;

	static const myBYTE JUST;
public:
	int Process(Selector selector);
	myfRGBQUAD ReturnColorFromString(string color);
	myBYTE ReturnFrontSizeFromString(string str);
	myBYTE ReturnLevelFromPX(string strpx);
	myBYTE ReturnLevelFromPercent(int percent);
	short ReturnFrontWeigthFromString(string str);
	myBYTE ReturnLevelFromEm(string strem);
	void ParserInline(void);
	int ProcessMargin(StyleSheet& css,string value);
	string GetPxFormEm(string em);

private:
	string DeleteNote(string& str);
};
#endif