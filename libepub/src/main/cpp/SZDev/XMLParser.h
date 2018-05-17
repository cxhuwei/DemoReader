#ifndef _XMLPARSER_H
#define _XMLPARSER_H

#include "SZDefine.h"
#include "tinyxml.h"
#include "ZlibFbreader.h"
#include "Book.h"
#include "ZLFile.h"


class CXMLParser
{
public:
	CXMLParser(void);
	~CXMLParser(void);
 	int Parser(myBYTE* buffer);
// 	int ParseContainer(CZlibFbreader* fb,byte* buffer);
// 	int ParseRootFile(CZEpubReader* book,byte* buffer);
// 	int ParseObepsFile(CZEpubReader* book,byte* buffer);
	int ParseContainer(CZEpubReader* fb,CZLFile* zFile);
	int ParseRootFile(CZEpubReader* book,CZLFile* zFile);
	int ParseObepsFile(CZEpubReader* book,CZLFile* zFile);
	string obeps;
private:
	TiXmlDocument m_xmlDocument;
	map<string,string> m_id_html;
	vector<string> m_idref;
	bool ismanifest;
	bool isspine;
	string m_coverID;
	string m_oebpsdir;
	int m_encodeType;//0-gbk 1-utf8 2- gb18080 3- other
public:
	void putOebpsDir(string oebpsdir);
	void parseElement(TiXmlNode* pElem,CZEpubReader* object,int type=0);//type=0:opf文件;type=1:ncx文件
	void ParseTextElement(TiXmlNode* pParent,CZEpubReader* object,int type);//
	void ParseAttElement(TiXmlNode* pParent,CZEpubReader* object,int type);
	void ParseChapterElement(TiXmlNode* pParent,CZEpubReader* object);
	void ParsePoint(TiXmlNode* pCurrent,CZEpubReader* object,int level=1);
	int ProcessSpine(CZEpubReader* book);
};

#endif