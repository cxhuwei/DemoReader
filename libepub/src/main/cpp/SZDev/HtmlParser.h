#if !defined(HTMLPARSER_H)
#define HTMLPARSER_H

#include "../SZDev/SZDefine.h"
/*#include "ByteBuffer.h"*/
#include "StyleSheetParser.h"
#include <cstring>

class HTMLNodeProp
{
public:
	HTMLNodeProp()
	{
		szName = szValue = NULL;
	}
	~HTMLNodeProp()
	{
		Clear();
	}
	HTMLNodeProp(const HTMLNodeProp& hnp)
	{
		int nameLength = strlen(hnp.szName);
		szName = new char[nameLength + 1];
		if (szName != NULL)
		{
			memset(szName,0,nameLength + 1);
			memcpy(szName,hnp.szName,nameLength);
		}
		
		int valueLength = strlen(hnp.szValue);
		szValue = new char[valueLength + 1];
		if (szValue != NULL)
		{
			memset(szValue,0,valueLength + 1);
			memcpy(szValue,hnp.szValue,valueLength);
		}
		
	}
	void HTMLNodePropName(string& bytebuffer)
	{
		if (szName != NULL)
		{
			delete[] szName;
			szName = NULL;
		}
		
		szName = new char[bytebuffer.size() + 1];
		if (szName != NULL)
		{
			memset(szName,0,bytebuffer.size() + 1);
			memcpy(szName,bytebuffer.c_str(),bytebuffer.size());
		}
				
	}
	void HTMLNodePropValue(string& bytebuffer)
	{
		if (szValue!= NULL)
		{
			delete[] szValue;
			szValue = NULL;
		}
		
		szValue = new char[bytebuffer.size() + 1];
		if (szValue != NULL)
		{
			memset(szValue,0,bytebuffer.size() + 1);
			memcpy(szValue,bytebuffer.c_str(),bytebuffer.size());
		}
		
	}
	HTMLNodeProp& operator = (const HTMLNodeProp& hnp)
	{
		Clear();
		int nameLength = strlen(hnp.szName);
		szName = new char[nameLength + 1];
		if (szName != NULL)
		{
			memset(szName,0,nameLength + 1);
			memcpy(szName,hnp.szName,nameLength);
		}
		
		int valueLength = strlen(hnp.szValue);
		szValue = new char[valueLength + 1];
		if (szValue != NULL)
		{
			memset(szValue,0,valueLength + 1);
			memcpy(szValue,hnp.szValue,valueLength);
		}
		
		return *this;
	}
	bool operator == (const HTMLNodeProp& hnp)
	{
		if (strcmp(hnp.szName,this->szName) == 0)
		{
			if (strcmp(hnp.szValue,this->szValue) == 0)
			{
				return true;
			}
			else
				return false;
		}
		else
			return false;
	}
	void Clear()
	{
		if (szName != NULL)
		{
			delete[] szName;
			szName = NULL;
		}
		if (szValue != NULL)
		{
			delete[] szValue;
			szValue = NULL;
		}
	}
 	char* szName;//标签属性名
	char* szValue;//属性值
};

class HTMLNode
{
public:
	HTMLNode()
	{
		memset(&nodeType,0,sizeof(HTMLNodeType));
		memset(&tagType,0,sizeof(HTMLTagType));	 
		nodeProps	= NULL;
		tagName		= NULL;
		text		= NULL;
		propCount	= 0;		
	}
	HTMLNodeType nodeType;
	HTMLTagType  tagType;
	HTMLNodeProp* nodeProps;
	char		* tagName;//标签名
	myWcharT		* text;
	int			propCount;
};


typedef std::map<const std::string, HTMLTagType> mapHTMLTagType; 
typedef std::vector<HTMLNode> VtHTMLNode;
typedef VtHTMLNode::iterator   VtHTMLNodeIterator;
class CHtmlParser
{ 
public:	
	CHtmlParser();
	/*CHtmlParser(CZLFile* file);*/
	CHtmlParser(myBYTE* szHtml,int len,VtHTMLNode* htmlNode);
	~CHtmlParser(void);
	void InitMapTagType();
	bool ParserHtml(void);
	void TextData(myBYTE* buffer,int start, int length);
	void TextData(string buff);
	void ProcessStartTag(/*CByteBuffer*/string& tagName, std::vector<HTMLNodeProp>& v_attributes);
	void Clear(std::vector<HTMLNodeProp>& vhnp);
	HTMLTagType GetTagTypeByTagName(/*const char**/string tagName);
	void ProcessEndTag(string&  tagName, std::vector<HTMLNodeProp>& v_attributes);
	void ProcessFullTag(/*CByteBuffer*/string& tagName, std::vector<HTMLNodeProp>& v_attributes);
	void ProcessTag(/*CByteBuffer*/string& tagName, std::vector<HTMLNodeProp>& v_attributes, HTMLNodeType nodeType,const char* text,int textlength);	
	Encodings GetEncoding(char* str);
	std::string ProcessEntity(myBYTE* buffer,int start, int length);
private:
	static const myBYTE START_DOCUMENT;
	static const myBYTE START_TAG ;
	static const myBYTE END_TAG ;
	static const myBYTE TEXT ;
	static const myBYTE COMMENT ;
	static const myBYTE LANGLE ;
	static const myBYTE WS_AFTER_START_TAG_NAME ;
	static const myBYTE WS_AFTER_END_TAG_NAME ;
	static const myBYTE WAIT_EQUALS ;
	static const myBYTE WAIT_ATTRIBUTE_VALUE ;
	static const myBYTE SLASH ;
	static const myBYTE ATTRIBUTE_NAME ;
	static const myBYTE S_ATTRIBUTE_VALUE ;
	static const myBYTE DEFAULT_ATTRIBUTE_VALUE ;
	static const myBYTE COMMENT_MINUS ;
	static const myBYTE D_ATTRIBUTE_VALUE ;
	static const myBYTE SCRIPT ;
	static const myBYTE ENTITY_REF ;
	static const myBYTE STYLE;
	static const myBYTE STYLECONTENT;
	//attributer
	myBYTE	*	m_szHtml;
	int		m_dataLength;	
	bool		m_isFindEncoding;
	Encodings	m_encoding;	//encoding
	
	mapHTMLTagType m_mapTagType;
	std::map<std::string,std::string> m_entity;
public:
	VtHTMLNode  *m_vHtmlNode;
	map<string,StyleSheet> m_inlineMCSS;//网页内部css
	void PutData(myBYTE* szHtml, int len);//
	/*void PutData(CZLFile* file);*/
	void ClearData();
	bool ParserHtml2(void);
	
private:
	std::vector<std::string> m_vCssFilePath;
	bool m_isHaveCSS;	
public:
	bool IsHaveCSS(void);
	std::vector<std::string> ReturnCSSFilePath(void);

//锚点相关
private:
	int m_anchorNumber;
public:
	void PutAnchor(map<string,int>* pMSI);
	map<string,int>* m_PmAnchorNameContentNumber;
	// html里的title
	myWcharT* m_title;
private:
	bool m_bodystart;
	bool m_isH;
	bool m_isCurTitle;
	bool m_isBody;
	HTMLNodeType m_lastNodeType;
	char m_lastNodeName[16];
};
#endif // !defined(HTMLPARSER_H)
