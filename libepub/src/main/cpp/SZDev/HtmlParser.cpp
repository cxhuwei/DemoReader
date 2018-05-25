#include "StdAfx.h"
#include "HtmlParser.h"
#include "StringConver.h"
#include "ZLFile.h"
#include "StyleSheetParser.h"
#include "MAlgorithm.h"

const myBYTE CHtmlParser::START_DOCUMENT = 0;
const myBYTE CHtmlParser::START_TAG = 1;
const myBYTE CHtmlParser::END_TAG = 2;
const myBYTE CHtmlParser::TEXT = 3;
const myBYTE CHtmlParser::COMMENT = 6;
const myBYTE CHtmlParser::LANGLE = 7;
const myBYTE CHtmlParser::WS_AFTER_START_TAG_NAME = 8;
const myBYTE CHtmlParser::WS_AFTER_END_TAG_NAME = 9;
const myBYTE CHtmlParser::WAIT_EQUALS = 10;
const myBYTE CHtmlParser::WAIT_ATTRIBUTE_VALUE = 11;
const myBYTE CHtmlParser::SLASH = 12;
const myBYTE CHtmlParser::ATTRIBUTE_NAME = 13;
const myBYTE CHtmlParser::S_ATTRIBUTE_VALUE = 14;
const myBYTE CHtmlParser::DEFAULT_ATTRIBUTE_VALUE = 15;
const myBYTE CHtmlParser::COMMENT_MINUS = 17;
const myBYTE CHtmlParser::D_ATTRIBUTE_VALUE = 18;
const myBYTE CHtmlParser::SCRIPT = 19;
const myBYTE CHtmlParser::ENTITY_REF = 20;
const myBYTE CHtmlParser::STYLE = 21;
const myBYTE CHtmlParser::STYLECONTENT = 22;
CHtmlParser::CHtmlParser(myBYTE* szHtml,int len,VtHTMLNode* htmlNode): m_isFindEncoding(false),m_encoding(UTF8)
		, m_isHaveCSS(false)
		, m_anchorNumber(-1)
		, m_isCurTitle(false)
		, m_isBody(false)
{
	InitMapTagType();
	m_szHtml = szHtml;
	m_dataLength = len;
	m_vHtmlNode = htmlNode;
	m_title = NULL;
}
CHtmlParser::CHtmlParser(): m_isFindEncoding(false),m_encoding(UTF8)
		, m_isHaveCSS(false)
		, m_anchorNumber(-1)
		, m_isCurTitle(false)
		, m_isBody(false)
		,m_isH(false)
{
	InitMapTagType();
	m_szHtml = NULL;
	m_dataLength = 0;
	m_title = NULL;
}

// CHtmlParser::CHtmlParser( CZLFile* file ): m_isFindEncoding(false),m_encoding(UNKNOWN)
// {
// 	InitMapTagType();
// 	m_szHtml = file->GetFileData();
// 	m_dataLength = file->GetDataLength();
// }

CHtmlParser::~CHtmlParser(void)
{
	ClearData();
}

void CHtmlParser::InitMapTagType()
{
	m_mapTagType.insert(std::pair<string,HTMLTagType>("a",TAG_A));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("b",TAG_B));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("br",TAG_BR));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("div",TAG_DIV));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("font",TAG_FONT));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("h1",TAG_H1));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("h2",TAG_H2));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("h3",TAG_H3));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("h4",TAG_H4));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("h5",TAG_H5));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("h6",TAG_H6));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("hr",TAG_HR));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("i",TAG_I));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("img",TAG_IMG));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("image",TAG_IMG));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("span",TAG_SPAN));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("script",TAG_SCRIPT));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("style",TAG_STYLE));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("p",TAG_P));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("ul",TAG_UL));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("li",TAG_LI));
	m_mapTagType.insert(std::pair<string,HTMLTagType>("sup",TAG_SUP));

	m_entity.insert(pair<string,string>("&nbsp;"," "));
	m_entity.insert(pair<string,string>("&lt;","<"));
	m_entity.insert(pair<string,string>("&gt;",">"));
	m_entity.insert(pair<string,string>("&amp;","&"));
	m_entity.insert(pair<string,string>("&quot;","\""));
	m_entity.insert(pair<string,string>("&times;","×"));
	m_entity.insert(pair<string,string>("&divide;","÷"));

	m_entity.insert(pair<string,string>("&#160;"," "));
	m_entity.insert(pair<string,string>("&#60;","<"));
	m_entity.insert(pair<string,string>("&#62;",">"));
	m_entity.insert(pair<string,string>("&#38;","&"));
	m_entity.insert(pair<string,string>("&#34;","\""));
	m_entity.insert(pair<string,string>("&#215;","×"));
	m_entity.insert(pair<string,string>("&#247;","÷"));


}

bool CHtmlParser::ParserHtml(void)
{

	if (m_szHtml == NULL )
	{
		return false;
	}

	m_lastNodeType = NODE_CLOSE_TAG;
	strcpy(m_lastNodeName,"body");
	m_isH = false;
	char* body =strstr((char*)m_szHtml,"body");
	if (body != NULL)
	{
		m_isBody = true;
		m_bodystart = false;
	}
	else
	{
		m_isBody = false;
		m_bodystart = true;
	}
	m_anchorNumber = -1;
	int HtmlOffset(0);
	int offset(0);//下标偏移量
	int startPosition(0);
	bool scriptOpened = false;
	bool style = false;
	char* s_tag=0;
	char* e_tag=0;
	char* s_script=0;//script区域指示
	char* e_script=0;
	myBYTE state = START_DOCUMENT;
	/*CByteBuffer*/string tagName;
	/*CByteBuffer*/string attributeName;
	/*CByteBuffer*/string attributeValue;
	/*CByteBuffer*/string textData;
	vector<HTMLNodeProp> attributes;
	HTMLNodeProp t_attribute;



	for(int i = -1;i<m_dataLength;)
	{
		mainSwitchLabel:
		switch(state)
		{
			case START_DOCUMENT:
				while(m_szHtml[++i] != '<'){}
				state = LANGLE;
				break;
			case LANGLE:

				//offset = HtmlOffset + i;
				switch(m_szHtml[++i])
				{
					case '/':
						state = END_TAG;
						startPosition = i + 1;
						break;
					case '!':
						//{
						switch(m_szHtml[++i])
						{
							case '-':
								state = COMMENT_MINUS;
								i--;
								break;
							default:
								state = COMMENT;
								break;
						}
						//}
						break;
					case '?':
						state = COMMENT;
						break;
					default:
						state = START_TAG;
						startPosition = i;
						break;
				}
				break;
			case SCRIPT:
// 			while(true)
// 			{
				if (m_szHtml[++i] == '<')
				{
					if (m_szHtml[++i] == '/')
					{
						state = END_TAG;
						startPosition = i + 1;
						//break;
						//goto mainSwitchLabel;
					}
				}
				break;
				//	}
			case COMMENT_MINUS:
			{
				int minusCounter = 0;
				while(minusCounter != 2)
				{
					switch(m_szHtml[++i])
					{
						case '-':
							minusCounter++;
							break;
						default:
							minusCounter = 0;
							break;
					}
				}
				switch(m_szHtml[++i])
				{
					case '>':
						state = TEXT;
						startPosition = i + 1;
						goto mainSwitchLabel;
				}
			}
			case COMMENT:
// 			while(true)
// 			{
				switch(m_szHtml[++i])
				{
					case '>':
						if (!m_isFindEncoding)
						{
							char* temp = new char[i - startPosition + 1];
							if (temp)
							{
								memcpy(temp,m_szHtml+startPosition,i-startPosition);
								temp[i - startPosition] = 0;
								m_encoding = GetEncoding(temp);
							}
							else
								m_encoding = UNKNOWN;
							if (m_encoding != UNKNOWN)
							{
								m_isFindEncoding = true;
							}
							if (temp)
							{
								delete[] temp;
								temp = NULL;
							}
						}

						state = TEXT;
						startPosition = i + 1;
						//goto mainSwitchLabel;
						break;
				}
				break;
				//}
			case START_TAG:
			{
// 				while(true)
// 				{
				switch(m_szHtml[++i])
				{
					case 0x0008:
					case 0x0009:
					case 0x000A:
					case 0x000B:
					case 0x000C:
					case 0x000D:
					case ' ':
						state = WS_AFTER_START_TAG_NAME;
						tagName.append((char*)(m_szHtml + startPosition),i - startPosition);
						//tagName.Append(m_szHtml,startPosition,i - startPosition);
						/* add by myf at 2011-10-26
							for css */
						if (/*tagName.Equals("style")*/tagName == "style")
						{
							state = STYLE;
						}
						//goto mainSwitchLabel;
						break;
					case '>':
					{
						state = TEXT;
						tagName.append((char*)(m_szHtml + startPosition),i - startPosition);
						if (/*tagName.Equals("style")*/tagName == "style")
						{
							state = STYLE;
							break;
						}
						//tagName.Append(m_szHtml,startPosition,i - startPosition);
						ProcessStartTag(tagName,attributes);
						if (/*tagName.Equals("script")*/tagName == "script")
						{
							state = SCRIPT;
							scriptOpened = true;
							break;
						}
						//tagName.Clear();
						tagName.erase(tagName.begin(),tagName.end());
						startPosition = i + 1;
						break;

					}
					case '/':
					{
						state = SLASH;
						tagName.append((char*)(m_szHtml + startPosition),i - startPosition);
						//tagName.Append(m_szHtml,startPosition,i - startPosition);

						break;
					}
				}
				//}
				break;
			}
			case END_TAG:
// 			while(true)
// 			{
				switch(m_szHtml[++i])
				{
					case 0x0008:
					case 0x0009:
					case 0x000A:
					case 0x000B:
					case 0x000C:
					case 0x000D:
					case ' ':
						state = WS_AFTER_END_TAG_NAME;
						tagName.append((char*)(m_szHtml + startPosition),i - startPosition);
						//tagName.Append(m_szHtml,startPosition, i - startPosition);
						break;
					case '>':
						tagName.append((char*)(m_szHtml + startPosition),i - startPosition);
						//tagName.Append(m_szHtml,startPosition,i - startPosition);
						if (/*!tagName.Equals("style")*/tagName != "style")
						{
							ProcessEndTag(tagName,attributes);
						}


						if (/*tagName.Equals("script")*/tagName == "script")
						{
							scriptOpened = false;
						}
						else if (tagName == "html")
						{
							return true;
						}
						if (scriptOpened)
						{
							state = SCRIPT;
						}
						else
						{
							state = TEXT;
							startPosition = i + 1;
						}
						//tagName.Clear();
						tagName.erase(tagName.begin(),tagName.end());
						break;
				}
				break;
				//}
			case WS_AFTER_START_TAG_NAME:
				switch(m_szHtml[++i])
				{
					case '>':

						ProcessStartTag(tagName,attributes);
						if (/*tagName.Equals("script")*/tagName == "script")
						{
							state = SCRIPT;
							scriptOpened = true;
							break;

						}
						//tagName.Clear();
						tagName.erase(tagName.begin(),tagName.end());
						state = TEXT;
						startPosition = i + 1;
						break;
					case '/':
						state = SLASH;
						break;
					case 0x0008:
					case 0x0009:
					case 0x000A:
					case 0x000B:
					case 0x000C:
					case 0x000D:
					case ' ':
						break;
					default:
						state = ATTRIBUTE_NAME;
						startPosition = i;
						break;
				}
				break;
			case WS_AFTER_END_TAG_NAME:
				switch(m_szHtml[++i])
				{
					case '>':
					{
						if (/*!tagName.Equals("style")*/tagName != "style")
						{
							ProcessEndTag(tagName,attributes);
						}

						if (/*tagName.Equals("script")*/tagName == "script")
						{
							scriptOpened = false;
						}
						//tagName.Clear();
						tagName.erase(tagName.begin(),tagName.end());
					}
						if (scriptOpened)
						{
							state = SCRIPT;
						}
						else
						{
							state = TEXT;
							startPosition = i + 1;
						}
						break;
				}
				break;
			case ATTRIBUTE_NAME:
// 					while (true)
// 					{
				switch(m_szHtml[++i])
				{
					case '=':
						attributeName.append((char*)(m_szHtml + startPosition),i-startPosition);
						//attributeName.Append(m_szHtml,startPosition,i - startPosition);
						state = WAIT_ATTRIBUTE_VALUE;
						break;
					case 0x0008:
					case 0x0009:
					case 0x000A:
					case 0x000B:
					case 0x000C:
					case 0x000D:
					case ' ':
						attributeName.append((char*)(m_szHtml + startPosition),i-startPosition);
						//attributeName.Append(m_szHtml,startPosition,i - startPosition);
						state = WAIT_EQUALS;
						break;
						//goto mainSwitchLabel;
				}
				break;
				//}
			case WAIT_EQUALS:
// 					while(true)
// 					{
				switch(m_szHtml[++i])
				{
					case '=':
						state = WAIT_ATTRIBUTE_VALUE;
						//goto mainSwitchLabel;
				}
				break;
				//}
			case WAIT_ATTRIBUTE_VALUE:
// 					while(true)
// 					{
				switch(m_szHtml[++i])
				{
					case ' ':
						break;
					case '\t':
						break;
					case '\n':
						break;
					case '\'':
						state = S_ATTRIBUTE_VALUE;
						startPosition = i + 1;
						break;
						//goto mainSwitchLabel;
					case '"':
						state = D_ATTRIBUTE_VALUE;
						startPosition = i + 1;
						break;
						//goto mainSwitchLabel;
					default:
						state = DEFAULT_ATTRIBUTE_VALUE;
						startPosition = i + 1;
						break;
						//goto mainSwitchLabel;
				}
				break;
				//}
			case DEFAULT_ATTRIBUTE_VALUE:
// 					while(true)
// 					{
				i++;
				if (m_szHtml[i] == ' ' || m_szHtml[i] == '\'' || m_szHtml[i] == '"' || m_szHtml[i] == '>')
				{
					attributeValue.append((char*)(m_szHtml + startPosition),i-startPosition);
					//attributeValue.Append(m_szHtml,startPosition,i - startPosition);
					t_attribute.HTMLNodePropName(attributeName);
					t_attribute.HTMLNodePropValue(attributeValue);
					attributes.push_back(t_attribute);
					attributeName.erase(attributeName.begin(),attributeName.end());
					attributeValue.erase(attributeValue.begin(),attributeValue.end());
// 							attributeName.Clear();
// 							attributeValue.Clear();
				}
				switch(m_szHtml[i])
				{
					case ' ':
					case '\'':
					case '"':
						state = WS_AFTER_START_TAG_NAME;
						break;
						//goto mainSwitchLabel;
					case '/':
						state = SLASH;
						break;
						//goto mainSwitchLabel;
					case '>':
						ProcessStartTag(tagName,attributes);
						if (/*tagName.Equals("script")*/tagName == "script")
						{

							scriptOpened = true;
							state = SCRIPT;
							goto mainSwitchLabel;
						}

						//tagName.Clear();
						tagName.erase(tagName.begin(),tagName.end());
						state = TEXT;
						startPosition = i + 1;
						break;

				}
				break;
				//}
			case D_ATTRIBUTE_VALUE:
// 					while(true)
// 					{
				switch(m_szHtml[++i])
				{
					case '"':
						attributeValue.append((char*)(m_szHtml + startPosition),i-startPosition);
						//attributeValue.Append(m_szHtml,startPosition,i - startPosition);
						state = WS_AFTER_START_TAG_NAME;
						t_attribute.HTMLNodePropName(attributeName);
						t_attribute.HTMLNodePropValue(attributeValue);
						attributes.push_back(t_attribute);
						attributeName.erase(attributeName.begin(),attributeName.end());
						attributeValue.erase(attributeValue.begin(),attributeValue.end());
// 							attributeValue.Clear();
// 							attributeName.Clear();
						//goto mainSwitchLabel;
				}
				break;
				//	}
			case S_ATTRIBUTE_VALUE:
// 					while(true)
// 					{
				switch(m_szHtml[++i])
				{
					case '\'':
						attributeValue.append((char*)(m_szHtml + startPosition),i-startPosition);
						//attributeValue.Append(m_szHtml,startPosition,i - startPosition);
						state = WS_AFTER_START_TAG_NAME;
						t_attribute.HTMLNodePropName(attributeName);
						t_attribute.HTMLNodePropValue(attributeValue);
						attributes.push_back(t_attribute);
						//attributes.insert(make_pair(attributeName,attributeValue));
// 							attributeValue.Clear();
// 							attributeName.Clear();
						attributeName.erase(attributeName.begin(),attributeName.end());
						attributeValue.erase(attributeValue.begin(),attributeValue.end());
						//goto mainSwitchLabel;
				}
				break;
				//}
			case SLASH:
// 					while(true)
// 					{
				switch(m_szHtml[++i])
				{
					case ' ':
						break;
					case '>':
						state = TEXT;
						ProcessFullTag(tagName,attributes);
						//tagName.Clear();
						tagName.erase(tagName.begin(),tagName.end());
						startPosition = i + 1;
						break;
						//goto mainSwitchLabel;
					default:
						state = DEFAULT_ATTRIBUTE_VALUE;
						break;
						//goto mainSwitchLabel;
				}
				break;
				//}
			case TEXT:
// 					while(true)
// 					{
				switch(m_szHtml[++i])
				{
					case '<':
						if (i > startPosition)
						{
							if (textData.empty())
							{
								TextData(m_szHtml,startPosition,i - startPosition);
							}
							else
							{

								textData.append((char*)(m_szHtml + startPosition),i - startPosition - 1);
								TextData(textData);
							}
							textData.erase(textData.begin(),textData.end());
							//textData.Append(m_szHtml,startPosition,i - startPosition);
						}
						state = LANGLE;
						break;
					case '&':
						if (i > startPosition)
						{
							textData.append((char*)(m_szHtml + startPosition),i - startPosition);
						}
						state = ENTITY_REF;
						startPosition = i+1;
						break;
				}
				break;
			case ENTITY_REF:
				if (m_szHtml[++i] == ';')
				{
					string re = ProcessEntity(m_szHtml,startPosition,i - startPosition);
					if (re != "")
					{
						textData.append(re);
					}
					state = TEXT;
					startPosition = i + 1;
				}
				break;
			case STYLE:
				switch(m_szHtml[++i])
				{
					case '>':
						startPosition = i + 1;
						state = STYLECONTENT;
						break;
				}
				break;
			case STYLECONTENT:
				switch(m_szHtml[++i])
				{
					case 0x0008:
					case 0x0009:
					case 0x000A:
					case 0x000B:
					case 0x000C:
					case 0x000D:
						break;
					case '<':
						CStyleSheetParser css;
						css.PutData(m_szHtml + startPosition,i - startPosition);
						css.Parser();
						m_inlineMCSS = css.m_MCSS;
						state = LANGLE;
						break;

				}
				break;

		}
	}
	return true;
}

void CHtmlParser::TextData( myBYTE* buffer,int start, int length )
{
	/*CByteBuffer*/string temp;
	vector<HTMLNodeProp> v_temp;
	ProcessTag(temp,v_temp,NODE_CONTENT,(const char*)(buffer + start),length);
}

void CHtmlParser::TextData(string buff )
{
	string temp;
	vector<HTMLNodeProp> v_temp;
	ProcessTag(temp,v_temp,NODE_CONTENT,buff.c_str(),buff.size());
}

void CHtmlParser::ProcessStartTag(/*CByteBuffer*/string& tagName, vector<HTMLNodeProp>& v_attributes)
{
	if (tagName == "title")
	{
		m_isCurTitle = true;
	}
	ProcessTag(tagName,v_attributes,NODE_START_TAG,NULL,0);
}

void CHtmlParser::ProcessEndTag(/*CByteBuffer*/string&  tagName, vector<HTMLNodeProp>& v_attributes)
{
	if (tagName == "title")
	{
		m_isCurTitle = false;
	}
	ProcessTag(tagName,v_attributes,NODE_CLOSE_TAG,NULL,0);
}

void CHtmlParser::ProcessFullTag(/*CByteBuffer*/string& tagName, vector<HTMLNodeProp>& v_attributes)
{
	if (strcmp(/*tagName.ToString()*/tagName.c_str(),"link") == 0)
	{
		HTMLNodeProp finder;
		finder.szName = new char[16];
		strcpy(finder.szName,"type");
		finder.szValue = new char[16];
		strcpy(finder.szValue,"text/css");
		vector<HTMLNodeProp>::iterator pos = find(v_attributes.begin(),v_attributes.end(),finder);
		if (pos != v_attributes.end())
		{
			m_isHaveCSS = true;
		}
		else
			m_isHaveCSS = false;
		if (m_isHaveCSS)
		{
			for (int i=0;i<v_attributes.size();i++)
			{
// 				if (strcmp(v_attributes[i].szName,"rel") == 0)
// 				{
// 					if (strcmp(v_attributes[i].szValue,"stylesheet") == 0)
// 					{
// 						m_isHaveCSS = true;
// 					}
// 					else
// 						m_isHaveCSS = false;
// 				}
// 				else if (strcmp(v_attributes[i].szName,"type") == 0)
// 				{
// 					if (strcmp(v_attributes[i].szValue,"text/css") == 0)
// 					{
// 						m_isHaveCSS = true;
// 					}
// 					else
// 						m_isHaveCSS = false;
// 				}
				if (strcmp(v_attributes[i].szName,"href") == 0)
				{
					m_vCssFilePath.push_back(v_attributes[i].szValue);
				}
			}
		}
	}
	ProcessTag(tagName,v_attributes,NODE_FULL_TAG,NULL,0);
}

void CHtmlParser::Clear(vector<HTMLNodeProp>& vhnp)
{
	for (int i = 0; i < vhnp.size();i++)
	{
		vhnp[i].Clear();
	}
	vhnp.clear();
}

Encodings CHtmlParser::GetEncoding(char* str)
{
	char code[10] = {0};
	char* p = m_ToLower(str);
	memcpy(code,"gbk",3);
	if (strstr(p,code) != NULL)
	{
		return GBK;
	}
	memcpy(code,"utf-8",strlen("utf-8"));
	if (strstr(p,code) != NULL)
	{
		return UTF8;
	}
	memcpy(code,"gb2312",strlen("gb2312"));
	if (strstr(p,code) != NULL)
	{
		return GB2312;
	}
	memcpy(code,"gb18030",strlen("gb18030"));
	if (strstr(p,code) != NULL)
	{
		return GB18030;
	}
	return UTF8;
}

HTMLTagType CHtmlParser::GetTagTypeByTagName(/*const char**/string tagName)
{
	if (tagName.empty()/* == NULL*/)
	{
		return TAG_UNKNOWN;
	}
	string tagNameLow = m_ToLower(tagName);
	mapHTMLTagType::iterator iter= m_mapTagType.find(tagNameLow);
	if (iter == m_mapTagType.end())
	{
		return TAG_UNKNOWN;
	}
	return iter->second;
}

void CHtmlParser::ProcessTag(/*CByteBuffer*/string& tagName, vector<HTMLNodeProp>& v_attributes, HTMLNodeType nodeType,const char* text,int textlength)
{
	int i , j;
	int tagNameLength;
	int attributNumber;
	char* temp=NULL;
	bool isPush=true;

	CStringConver stringCon;
	HTMLNode node;
	node.nodeType = nodeType;
	bool tagtypeisH = false;

	node.tagType = GetTagTypeByTagName(/*tagName.ToString()*/tagName.c_str());
	tagNameLength = tagName.size()/*tagName.GetDataLength()*/;
	node.tagName = new char[tagNameLength + 1];
	if (nodeType == NODE_FULL_TAG || nodeType == NODE_CLOSE_TAG || nodeType == NODE_START_TAG)
	{
		if ((node.tagType == TAG_H1 || node.tagType == TAG_H2 || node.tagType == TAG_H3
			 || node.tagType == TAG_H4 || node.tagType == TAG_H5 || node.tagType == TAG_H6))
		{
			tagtypeisH  = true;
			if (nodeType == NODE_START_TAG)
			{
				m_anchorNumber++;
				m_isH = true;
			}
			else if (nodeType == NODE_CLOSE_TAG)
			{
				m_isH = false;
			}
		}
		if (node.tagType == TAG_IMG)
		{
			m_anchorNumber++;
		}
		if (node.tagName != NULL)
		{
			memset(node.tagName,0,tagNameLength + 1);
			if (/*tagName.ToString()*/tagName.c_str() != NULL)
			{
				memcpy(node.tagName,/*tagName.ToString()*/tagName.c_str(),tagNameLength);
			}
			else
				return;
		}
		if (m_isBody && strcmp(node.tagName,"body") == 0)
		{
			//m_isBody = true;
			m_bodystart = true;
		}

		attributNumber = v_attributes.size();
		node.propCount = attributNumber;
		if (attributNumber > 0)
		{
			node.nodeProps = new HTMLNodeProp[attributNumber];
			if (node.nodeProps != NULL)
			{
				for ( i = 0; i < attributNumber; i++)
				{
					node.nodeProps[i] = v_attributes[i];
					if (strcmp(v_attributes[i].szName,"id") == 0 || strcmp(v_attributes[i].szName,"name") == 0)
					{
						string anchorName(v_attributes[i].szValue);
						m_PmAnchorNameContentNumber->insert(pair<string,int>(anchorName,tagtypeisH?m_anchorNumber:m_anchorNumber+1));
					}
					if (m_isFindEncoding == false && strcmp(m_ToLower(tagName).c_str(),"meta") == 0)
					{
						char* p = m_ToLower(v_attributes[i].szName);
						if (strcmp(p,"content") == 0 || strcmp(p,"charset") == 0)
						{
							m_encoding = GetEncoding(v_attributes[i].szValue);
							if (m_encoding != UNKNOWN)
								m_isFindEncoding = true;
						}
					}
				}
			}
		}
		node.tagType = GetTagTypeByTagName(node.tagName);
	}
	else if (nodeType ==  NODE_CONTENT)
	{
		char* szText;
		szText = new char[textlength + 1];
		if (szText == NULL)
		{
			return;
		}
		memset(szText,0,textlength + 1);
		temp = new char[textlength + 1];
		if (temp == NULL)
		{
			return;
		}
		memset(temp,0,textlength + 1);
		memcpy(temp,text,textlength);
		for( i = 0,j = 0;i<textlength;i++)
		{
			if (temp[i] == '\r' || temp[i] == '\n'/* || temp[i] == ' '*/)
			{
				continue;
			}
			else
				szText[j++] = temp[i];
		}
		if (temp)
		{
			delete[] temp;
		}

		//如果是body体内的空白，取消掉

		if (m_lastNodeType != NODE_START_TAG || strcmp(m_lastNodeName,"body")==0)
		{
			m_Trim(szText);
		}
		if (strlen(szText) == 0)
		{
			isPush = false;
		}
		if (isPush && m_bodystart)
		{

			node.text = new myWcharT[textlength + 1];
			if (node.text == NULL)
			{
				return;
			}
			if (m_isH == false)
			{
				m_anchorNumber++;
			}
			memset(node.text,0,textlength*sizeof(myWcharT) + sizeof(myWcharT));
			switch(m_encoding)
			{
				case GBK:
				case GB2312:
					stringCon.GbkToUnicodeBig(szText,strlen(szText),node.text,textlength*sizeof(myWcharT));
					break;
				case UTF8:
					stringCon.Utf8ToUnicodeBig(szText,strlen(szText),node.text,textlength*sizeof(myWcharT));
					break;
				case GB18030:
					stringCon.Gb18030ToUnicodeBig(szText,strlen(szText),node.text,textlength*sizeof(myWcharT));
					break;
				default:
					memcpy(node.text,szText,textlength);
					break;
			}
		}
		if (szText)
		{
			delete[] szText;
			szText = NULL;
		}
	}
	m_lastNodeType = node.nodeType;
	strcpy(m_lastNodeName,node.tagName);
	if (isPush)
	{
// 		if (m_isCurTitle && nodeType == NODE_CONTENT)
// 		{
// 			m_title = node.text;
// 			if (node.tagName)
// 			{
// 				delete[] node.tagName;
// 				node.tagName = NULL;
// 			}
// 			m_anchorNumber--;
// 		}
// 		else
// 		{
		if (/*m_isBody*/m_bodystart)
		{

			m_vHtmlNode->push_back(node);
		}
		else
		{
			for (j=0;j<node.propCount;j++)
			{
				node.nodeProps[j].Clear();
			}
			if (node.nodeProps)
			{
				delete[] node.nodeProps;
				node.nodeProps = NULL;
			}
			if (node.tagName)
			{
				delete[] node.tagName;
				node.tagName = NULL;
			}
			if (node.text)
			{
				delete[] node.text;
				node.text = NULL;
			}
		}
//		}

	}

	else
	{
		if (node.tagName)
		{
			delete[] node.tagName;
			node.tagName = NULL;
		}
	}
	Clear(v_attributes);
}

void CHtmlParser::PutData(myBYTE* szHtml, int len)
{
	ClearData();
	m_szHtml = szHtml;
	m_dataLength = len;
}

// void CHtmlParser::PutData( CZLFile* file )
// {
// 	ClearData();
// 	m_szHtml = file->GetFileData();
// 	m_dataLength = file->GetDataLength();
// }

void CHtmlParser::ClearData()
{
// 	int i,j;
// 	int length = m_vHtmlNode->size();
// 	for (i = 0;i<length;i++)
// 	{
// 		/*m_vHtmlNode[i].nodeProps->Clear();*/
// 		for (j=0;j<m_vHtmlNode[i].propCount;j++)
// 		{
// 			m_vHtmlNode[i].nodeProps[j].Clear();
// 		}
// 		delete[] m_vHtmlNode[i].nodeProps;
// 		delete[] m_vHtmlNode[i].tagName;
// 		delete[] m_vHtmlNode[i].text;
// 	}
// 
// 	m_vHtmlNode->clear();
	m_mapTagType.clear();
	m_entity.clear();
	m_inlineMCSS.clear();
}

string CHtmlParser::ProcessEntity(myBYTE* buffer,int start, int length)
{
	string temp;
	if (strlen((char*)buffer) < start)
	{
		return string("");
	}
	temp.assign((char*)buffer,start - 1,length + 2);
	map<string,string>::iterator pos;
	for (pos = m_entity.begin(); pos != m_entity.end();pos++)
	{
		if (temp == pos->first)
		{
			break;
		}
	}
	if (pos != m_entity.end())
	{
		return pos->second;
	}
	else
		return string("");
}

bool CHtmlParser::ParserHtml2(void)
{

	return true;
}

bool CHtmlParser::IsHaveCSS(void)
{
	return m_isHaveCSS;
}

vector<string> CHtmlParser::ReturnCSSFilePath(void)
{
	return m_vCssFilePath;
}

void CHtmlParser::PutAnchor(map<string,int>* pMSI)
{
	m_PmAnchorNameContentNumber = pMSI;
}
