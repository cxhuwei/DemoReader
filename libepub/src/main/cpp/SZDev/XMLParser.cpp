#include "StdAfx.h"
#include "XMLParser.h"
#include "MAlgorithm.h"
#include "StringConver.h"

short int hexChar2dec(char c) {  
	if ( '0'<=c && c<='9' ) {  
		return short(c-'0');  
	} else if ( 'a'<=c && c<='f' ) {  
		return ( short(c-'a') + 10 );  
	} else if ( 'A'<=c && c<='F' ) {  
		return ( short(c-'A') + 10 );  
	} else {  
		return -1;  
	}  
}  
std::string deescapeURL( const string &URL )
{
	string result = "";  
	for ( unsigned int i=0; i<URL.size(); i++ ) {  
		char c = URL[i];  
		if ( c != '%' ) {  
			result += c;  
		} else {  
			char c1 = URL[++i];  
			char c0 = URL[++i];  
			int num = 0;  
			num += hexChar2dec(c1) * 16 + hexChar2dec(c0);  
			result += char(num);  
		}  
	}  
	return result;  
}
CXMLParser::CXMLParser(void)
{
	ismanifest = isspine = false;
	m_encodeType = 0;
}

CXMLParser::~CXMLParser(void)
{
	m_id_html.clear();
	m_idref.clear();
}

int CXMLParser::Parser(myBYTE* buffer)
{
	//m_xmlDocument("temp.xml");
	m_xmlDocument.Parse((char*)buffer);
	if ( m_xmlDocument.Error() )
	{
		return -1;
	}
	TiXmlDeclaration* p = m_xmlDocument.FirstChild()->ToDeclaration();
	char* encode = (char*)p->Encoding();
	char* lencode = m_ToLower(encode);
	if (strcmp(lencode,"utf-8") == 0)
	{
		m_encodeType = 1;
	}
	else if(strcmp(lencode,"gbk") == 0 || strcmp(lencode,"gb2312") == 0)
	{
		m_encodeType = 0;
	}
	else if (strcmp(lencode,"gb18030") == 0)
	{
		m_encodeType = 2;
	}
	else
		m_encodeType = 3;
	return 0;
}
int CXMLParser::ParseContainer(CZEpubReader* fb,CZLFile* zFile)
{
	if (Parser(zFile->GetFileData()) != 0)
	{
		return -1;
	}
	TiXmlElement *RootElement = m_xmlDocument.RootElement();
	TiXmlElement *rootfiles = RootElement->FirstChildElement();
	TiXmlElement *rootfile = rootfiles->FirstChildElement();
	TiXmlAttribute *fullpath = rootfile->FirstAttribute();
	for (;fullpath;fullpath = fullpath->Next())
	{
		if (strcmp(fullpath->Name(),"full-path") == 0)
		{
			fb->putRootFile(fullpath->Value());
			break;
		}
	}
	return fb->putRootFile(fullpath->Value());
	m_xmlDocument.Clear();
	return 0;
}
int CXMLParser::ParseRootFile(CZEpubReader* fb/*CZEpubReader* book*/,CZLFile* zFile)
{
	if (Parser(zFile->GetFileData()) != 0)
	{
		return -1;
	}
	TiXmlElement *RootElement = m_xmlDocument.RootElement();
	//TiXmlNode *pElement  = RootElement->FirstChild();
	parseElement(RootElement,fb);
	if (isspine && ismanifest)
	{
		ProcessSpine(fb);
	}
	return 0;

}
void CXMLParser::parseElement(TiXmlNode* pParent,CZEpubReader* object/*CZEpubReader* object*/,int type)
{
	if (NULL == pParent)
	{
		return;
	}
	TiXmlNode* pChild = pParent->FirstChild();
	for (;pChild;pChild = pChild->NextSibling())
	{
		int nType = pChild->Type();
		const char* name;
		switch(nType)
		{
		case TiXmlNode::ELEMENT:
			name = pChild->Value();
			if (strstr(name,"metadata") != NULL)
			{
				ParseTextElement(pChild,object,type);
			}
			else if (strstr(name,"manifest") != NULL || strstr(name,"spine") != NULL)
			{
				ParseAttElement(pChild,object,type);
			}
			else if (strstr(name,"navMap") != NULL)
			{
				ParseChapterElement(pChild,object);
			}
			break;
		case TiXmlNode::TEXT:
			//TiXmlNode* node = pChild->Parent();
			break;
		default:
			break;
		}
	}
	//
	
}
void CXMLParser::ParseTextElement(TiXmlNode* pParent,CZEpubReader* object,int type)
{
	if (NULL == pParent)
	{
		return;
	}
	TiXmlNode* pChild = pParent->FirstChild();
	for (;pChild;pChild = pChild->NextSibling())
	{
		int nType = pChild->Type();
		const char* name;
		if (nType == TiXmlNode::ELEMENT)
		{
			name = pChild->Value();
			if (type == 0)
			{//opf
				//CZEpubReader* pBook = object;
				object->m_book->m_bookInfo->PutEncoding(m_encodeType);
				if (strstr(name,"title") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->PutTitle(node->Value());
				
					
				}
				else if (strstr(name,"creator") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->PutCreator(node->Value());
				}
				else if (strstr(name,"subject") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->PutSubject(node->Value());
				}
				else if (strstr(name,"description") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putdescription(node->Value());
				}
				else if (strstr(name,"contributor") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putcontributor(node->Value());
				}
				else if (strstr(name,"date") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putdate(node->Value());
				}
				else if (strstr(name,"type") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Puttype(node->Value());
				}
				else if (strstr(name,"format") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putformat(node->Value());
				}
				else if (strstr(name,"identifier") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putidentifier(node->Value());
				}
				else if (strstr(name,"source") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putsource(node->Value());
				}
				else if (strstr(name,"language") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putlanguage(node->Value());
				}
				else if (strstr(name,"relation") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putrelation(node->Value());
				}
				else if (strstr(name,"coverage") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putcoverage(node->Value());
				}
				else if (strstr(name,"rights") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->Putrights(node->Value());
				}
				else if (strstr(name,"publisher") != NULL)
				{
					TiXmlNode* node = pChild->FirstChild();
					if (node == NULL)
					{
						continue;
					}
					object->m_book->m_bookInfo->PutPublisher(node->Value());
				}
				else if (strstr(name,"meta") != NULL)
				{
					TiXmlElement* item = pChild->ToElement();
					TiXmlAttribute* att = item->FirstAttribute();
					const char* attname = NULL;
					const char* content = NULL;
					bool isCover = true;
					while(att)
					{
						if (strcmp(att->Name(),"name") == 0)
						{
							attname = att->Value();
							if (strcmp(attname,"cover") != 0)
							{
								isCover = false;
								break;
							}
						}
						else if (strcmp(att->Name(),"content") == 0)
						{
							content = att->Value();
						}
						att = att->Next();
					}
					if (content != NULL && isCover)
					{
						m_coverID.assign(content);
					}
				}
				else
				{
					//continue;
				}
			}
			
		}
	}
}
void CXMLParser::ParseAttElement(TiXmlNode* pParent,CZEpubReader* object,int type)
{
	if (NULL == pParent)
	{
		return;
	}
// 	char* att1(NULL);
// 	char* att2(NULL);//目前只用得到两个属性
	string att1,att2;
	short attType;//0-manifest;1-spine;2-guide;
	string coverid,coveritem;//两种封面路径
	coverid = "";
	coveritem = "";
	if (strstr(pParent->Value(),"manifest") != NULL)
	{
		attType = 0;
		ismanifest = true;
	}
	else if (strstr(pParent->Value(),"spine") != NULL)
	{
		attType = 1;
		isspine = true;
	}
	else if (strstr(pParent->Value(),"guide") != NULL)
	{
		attType = 2;
	}
	bool isGetCoverFile(false);//是否进行封面提取
	if (!m_coverID.empty())
	{
		isGetCoverFile = true;
	}
	TiXmlNode* pChild = pParent->FirstChild();
	for (;pChild;pChild = pChild->NextSibling())
	{
		int nType = pChild->Type();
		if (nType == TiXmlNode::ELEMENT)
		{
			if (type == 0)
			{
				TiXmlElement* item = pChild->ToElement();
				TiXmlAttribute* att = item->FirstAttribute();
				while(att)
				{
					if (strcmp(att->Name(),"href") == 0)
					{
						att2.assign(att->Value());
					}
					else if (strcmp(att->Name(),"id") == 0)
					{
						att1.assign(att->Value());
						
					}
					if (strcmp(att->Name(),"idref") == 0 || strcmp(att->Name(),"type") == 0)
					{
						if (att1.empty())
						{
							att1.assign(att->Value());
						}
						else
						{
							att2.assign(att->Value());
						}
					}
					att = att->Next();
				}
				switch(attType)
				{
				case 0:
					att2.insert(0,m_oebpsdir);
					m_id_html.insert(std::pair<string,string>(att1,att2));
					//提取ncx
                    if (att1.compare("ncx") == 0 || att1.compare("toc") == 0)
					{
						object->PutNcxFile(att2);
					}
					//提取cover
					if (att1 == m_coverID)
					{
						//object->m_book->PutCoverFile(att2);
						coverid = att2;
					}
					
					if (att1.find("cover") != string::npos)
					{
						//object->m_book->PutCoverFile(att2);
                        if (att2.find("htm") == string::npos) {
                            coveritem = att2;
                        }
					}
					if (coverid == "")
					{
						object->m_book->PutCoverFile(coveritem);
					}
					else
						object->m_book->PutCoverFile(coverid);
					//提前bookname
					if (att1.find("bookname") != string::npos)
					{
						object->m_book->PutBookName(att2);
					}
					break;
				case 1:
					m_idref.push_back(att1);
					break;
				case 2:
					break;
				}
				if (!att1.empty())
				{
					att1.assign("");
				}
				if (!att2.empty())
				{
					att2.assign("");
				}
				
			}
		}
	}
}

int CXMLParser::ProcessSpine(CZEpubReader* book)
{
	int length = m_idref.size();
	for (int i = 0;i < length; i++)
	{
		map<string,string>::iterator pos;
		pos = m_id_html.find(m_idref[i]);
		if ( pos!= m_id_html.end())
		{		
			book->m_book->m_bookSpine->PushBackSpine(pos->second);
		}
		
	}
	return 0;
}

void CXMLParser::putOebpsDir( std::string oebpsdir )
{
	m_oebpsdir = oebpsdir;
	if (oebpsdir != "")
	{
		m_oebpsdir.append("/");
	}
	
}

int CXMLParser::ParseObepsFile( CZEpubReader* book,CZLFile* zFile )
{
	if (Parser(zFile->GetFileData()) == -1)
	{
		return -1;
	}
	TiXmlElement *RootElement = m_xmlDocument.RootElement();
	parseElement(RootElement,book);
	return 0;
}

void CXMLParser::ParseChapterElement( TiXmlNode* pParent,CZEpubReader* object )
{
	TiXmlNode* p_navPoint = pParent->FirstChild();
	while(p_navPoint)
	{
		if (strcmp(p_navPoint->Value(),"navPoint") != 0)
		{
            p_navPoint = p_navPoint->NextSibling();
			continue;
		}
		ParsePoint(p_navPoint,object);
		p_navPoint = p_navPoint->NextSibling();
	}
}

void CXMLParser::ParsePoint( TiXmlNode* pCurrent,CZEpubReader* object,int level )
{
	
	string order,chapter,file;
	//取order
	TiXmlAttribute* att = pCurrent->ToElement()->FirstAttribute();
	while(att)
	{
		if (strcmp(att->Name(),"playOrder") == 0)
		{
			order.assign(att->Value());
			break;
		}
		att = att->Next();
	}
	//
	TiXmlNode* p_navLabel = pCurrent->FirstChild();
	while(p_navLabel)
	{
		if (strcmp(p_navLabel->Value(),"navLabel") == 0)
		{
			TiXmlNode* text = p_navLabel->FirstChild();
			if (text != NULL)
			{
				if (text->Type() == TiXmlNode::ELEMENT)
				{
					TiXmlNode* temp = text->FirstChild();
					if (temp == NULL)
					{
						chapter = "";
					}
					else
						chapter.assign(temp->Value());
				}
				else if (text->Type() == TiXmlNode::TEXT)
				{
					chapter.assign(text->Value());
				}
			}

		}
		else if (strcmp(p_navLabel->Value(),"content") == 0)
		{
			TiXmlAttribute* att = p_navLabel->ToElement()->FirstAttribute();
			while(att)
			{
				if(strcmp(att->Name(),"src") == 0)
				{

					file.assign(att->Value());
					file = deescapeURL(file);
					file.insert(0,m_oebpsdir);
					break;
				}
				att = att->Next();
			}
		}
		else if (strcmp(p_navLabel->Value(),"navPoint") == 0)
		{
			ParsePoint(p_navLabel,object,level+1);
		}
		p_navLabel = p_navLabel->NextSibling();
	}
	int inlen = chapter.length();
	wchar_t* outText = new wchar_t[inlen+1];//最终在m_bookChapter类中clear方法中释放内存
	memset(outText,0,(inlen+1)*sizeof(wchar_t));
	int outlen = inlen*sizeof(wchar_t);
	CStringConver sc;
	switch (m_encodeType)
	{
	case 0:
		sc.GbkToUnicodeBig((char*)chapter.c_str(),inlen,outText,outlen);
		break;
	case 1:
		sc.Utf8ToUnicodeBig((char*)chapter.c_str(),inlen,outText,outlen);
		break;
	case 2:
		sc.Gb18030ToUnicodeBig((char*)chapter.c_str(),inlen,outText,outlen);
		break;
	default:
		break;
	}
	object->m_book->m_bookChapter->PutInfo(atoi(order.c_str()),outText,file,level);
}
