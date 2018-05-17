#include "StdAfx.h"
#include "StyleSheetParser.h"
#include "MAlgorithm.h"
#include "CSSColorMap.h"
#include <cstring>

const myBYTE CStyleSheetParser::START_DOCUMENT = 0;
const myBYTE CStyleSheetParser:: ANNOTATE = 1;
const myBYTE CStyleSheetParser:: START_TAG = 2;
const myBYTE CStyleSheetParser:: END_TAG = 3;
const myBYTE CStyleSheetParser:: TAG = 4;
const myBYTE CStyleSheetParser:: CLASS = 5;
const myBYTE CStyleSheetParser:: ID = 6;
const myBYTE CStyleSheetParser:: ATTR = 7;
const myBYTE CStyleSheetParser:: ATTRIBUTE_NAME = 8;
const myBYTE CStyleSheetParser:: ATTRIBUTE_VAULE = 9;
const myBYTE CStyleSheetParser:: JUST = 10;




CStyleSheetParser::CStyleSheetParser(void)
{
	m_cssData = NULL;
	m_cssLength = 0;
}

CStyleSheetParser::~CStyleSheetParser(void)
{
// 	if (m_cssData != NULL)
// 	{
// 		delete[] m_cssData;
// 		m_cssData = NULL;
// 	}
	Reset();
}

void CStyleSheetParser::PutData( myBYTE* data,int length )
{
	if (data != NULL)
	{
		m_cssData = data;
	}
	if (length > 0)
	{
		m_cssLength = length;
	}
}

int CStyleSheetParser::Parser()
{
	if (m_cssData == NULL)
	{
		return -1;
	}
	string attrname;//属性名
	string attrvalue;//属性值
	string tagname;//标签名
	//Attributes attrs; 
	Selector select;
	int startPosition(0);
	myBYTE selectorType;
	myBYTE state = START_DOCUMENT;
	for (int i = -1;i < m_cssLength;)
	{
		switch(state)
		{
		case START_DOCUMENT:
			switch(m_cssData[++i])
			{
			case 0x0008:
			case 0x0009:
			case 0x000A:
			case 0x000B:
			case 0x000C:
			case 0x000D:
			case ' ':
				break;
			case '@'://注释
				state = ANNOTATE;
				startPosition = i + 1;
				break;
			default:
				state = START_TAG;
				startPosition = i;
				break;
			}
			break;
		case ANNOTATE:
			while(1){
				if (m_cssData[++i] == ';'|| i == m_cssLength)
				{
					break;
				}
			}
			state = END_TAG;
			break;
		case START_TAG:
			switch(m_cssData[i])
			{
			case '.':
				state = CLASS;
				startPosition = i /*+ 1*/;
				break;
			case '#':
				state = ID;
				startPosition = i/* + 1*/;
				break;
			default:
				state = TAG;
				startPosition = i;
				break;
			}
			break;
		case END_TAG:
			switch(m_cssData[++i])
			{
			case 0x0008:
			case 0x0009:
			case 0x000A:
			case 0x000B:
			case 0x000C:
			case 0x000D:
			case ' ':	
				break;
			default:
				//process();//处理这个属性结
				state = START_TAG;
				startPosition = i;
				break;
			}
			break;
		case TAG:
			if (m_cssData[++i] == '{')
			{
				tagname.append((char*)(m_cssData + startPosition),i - startPosition);
				state = ATTRIBUTE_NAME;
				startPosition = i + 1;
				selectorType = SELECTOR_TYPE_TAG;
			}
			break;
		case CLASS:
			if (m_cssData[++i] == '{')
			{
				tagname.append((char*)(m_cssData + startPosition),i - startPosition);
				state = ATTRIBUTE_NAME;
				startPosition = i + 1;
				selectorType = SELECTOR_TYPE_CLASS;
			}
			break;
		case ID:
			if (m_cssData[++i] == '{')
			{
				tagname.append((char*)(m_cssData + startPosition),i - startPosition);
				state = ATTRIBUTE_NAME;
				startPosition = i + 1;
				selectorType = SELECTOR_TYPE_ID;
			}
			break;
		case ATTRIBUTE_NAME:
			switch(m_cssData[++i])
			{
			case 0x0008:
			case 0x0009:
			case 0x000A:
			case 0x000B:
			case 0x000C:
			case 0x000D:
			case ' ':	
				break;
			case '}':
				state = END_TAG;
				select.selectorType = selectorType;
				select.selectorName = m_ToLower(tagname);
				tagname.erase();
				//m_vSelector.push_back(select);
				Process(select);
				select.attrs.clear();
				break;
			case ':':
				attrname.append((char*)(m_cssData + startPosition),i - startPosition);
				state = ATTRIBUTE_VAULE;
				startPosition = i + 1;
				break;
					
			}
			break;
// 
// 			while(m_cssData[++i] != ':'){}
// 			attrname.append((char*)(m_cssData + startPosition),i - startPosition);
// 			state = ATTRIBUTE_VAULE;
// 			startPosition = i + 1;
// 			break;
		case ATTRIBUTE_VAULE:
			switch(m_cssData[++i])
			{
			case 0x0008:
			case 0x0009:
			case 0x000A:
			case 0x000B:
			case 0x000C:
			case 0x000D:
			case ' ':	
				break;
			case ';':
				attrvalue.append((char*)(m_cssData + startPosition),i - startPosition);
				m_ToLower(attrvalue);
				m_ToLower(attrname);
				select.attrs.insert(std::pair<string,string>(attrname,attrvalue));
				attrname.erase();
				attrvalue.erase();
				state = JUST;
				break;
			case '}':
				attrvalue.append((char*)(m_cssData + startPosition),i - startPosition);
				m_ToLower(attrvalue);
				m_ToLower(attrname);
				select.attrs.insert(std::pair<string,string>(attrname,attrvalue));
				attrname.erase();
				attrvalue.erase();
				state = END_TAG;
				select.selectorType = selectorType;
				select.selectorName = m_ToLower(tagname);
				tagname.erase();
				Process(select);
				//m_vSelector.push_back(select);
				select.attrs.clear();
				break;
			}
			break;
		case JUST:
			while (1)
			{
				if (m_cssData[i] != 0x0008
					&& m_cssData[i] != 0x0009
					&& m_cssData[i] != 0x000A
					&& m_cssData[i] != 0x000B
					&& m_cssData[i] != 0x000C
					&& m_cssData[i] != 0x000D
					&& m_cssData[i] != ' ')
				{
					break;
				}
				else
				{
					i++;
					continue;
				}
			}
			if (m_cssData[++i] == '}')
			{
				state = END_TAG;
				select.selectorType = selectorType;
				select.selectorName = m_ToLower(tagname);
				tagname.erase();
				Process(select);
				//m_vSelector.push_back(select);
				select.attrs.clear();
				
			}
			else
			{
				state = ATTRIBUTE_NAME;
				startPosition = i;
			}
			break;
		}
	}
	return 0;
}

void CStyleSheetParser::Reset()
{
// 	m_ClassName.erase();
// 	m_IDName.erase();
// 	m_TagName.erase();
// 	int i;
// 	for (i=0;i<m_vSelector.size();i++)
// 	{
// 		m_vSelector[i].selectorName.erase();
// 		m_vSelector[i].attrs.clear();
// 	}
// 	m_vSelector.clear();
	m_MCSS.clear();
}

int CStyleSheetParser::Process(Selector selector)
{//处理css.提取有用信息.
	string tempString = DeleteNote(selector.selectorName);
	string selectorName = m_TrimString(tempString);
	StyleSheet css;
	css.cssType = selector.selectorType;
	css.isBackGroundColor = css.isFontColor = false;
	css.ismargin = false;
	if (selectorName == "a")
	{
		css.UnderlineIsVaild = true;
		css.isUnderline = true;
	}
	Attributes::iterator pos;
	for (pos = selector.attrs.begin(); pos != selector.attrs.end();pos++)
	{

		string name = pos->first;
		string value = pos->second;
		m_TrimString(name);
		m_TrimString(value);
		if (name.find("background-color") != string::npos)
		{
			css.backGroundColor = ReturnColorFromString(value);
			css.isBackGroundColor = true;
		}
		//字体方面
		else if (name.find("color") != string::npos)
		{
			css.fontColor = ReturnColorFromString(value);
			css.isFontColor = true;
		}
// 		else if (name.find("font-family") != string::npos)
// 		{
// 			css.fontfamily = value;
// 		}
		else if (name.find("font-size") != string::npos)
		{
			css.fontSize = ReturnFrontSizeFromString(value);
		}
		else if (name.find("font-style") != string::npos)
		{
			if (value == "italic" || value == "oblique")
			{
				css.isItalic = true;
			}
			else
				css.isItalic = false;
		}
		else if (name.find("text-shadow") != string::npos)
		{
		}
		else if (name.find("font-weight") != string::npos)
		{
			if (ReturnFrontWeigthFromString(value) >= 7)
			{
				css.Bold = 1;
			}
			else
				css.Bold = 0;
		}
		else if (name.find("text-decoration") != string::npos)
		{
			if (value == "underline")
			{
				css.UnderlineIsVaild = true;
				css.isUnderline = true;
			}
			else
			{
				css.UnderlineIsVaild = true;
				css.isUnderline = false;
			}
		}
		//布局
		else if (name.find("text-align") != string::npos)
		{
			css.textAlignIsVaild = true;
			if (value.find("center") != string::npos)
				css.textAlign = CENTER;
			else if (value.find("left") != string::npos)
			{
				css.textAlign = LEFT;
			}
			else if ((int)value.find("right") != string::npos)
			{
				css.textAlign = RIGHT;
			}
		}
		else if (name.find("margin") != string::npos)
		{
			ProcessMargin(css,value);
		}
		
	}
	m_MCSS.insert(std::pair<string,StyleSheet>(selectorName,css));
	return 0;
}

myfRGBQUAD CStyleSheetParser::ReturnColorFromString(string color)
{
	myfRGBQUAD rgb;
	rgb.rgbBlue = rgb.rgbGreen = rgb.rgbRed = 0;
	rgb.rgbReserved = 0;
	const char* szColor = color.c_str();	
	if (color.find("rgb") != string::npos)
	{
		size_t s = color.find('(');
		size_t e = color.find(')');
		if (s != string::npos && e != string::npos)
		{
			string sub = color.substr(s + 1,e-s -1);
			char* str = (char*)sub.c_str();
			char* co = strtok(str,",");
			if (co != NULL)
			{
				int r = atoi(co);
				rgb.rgbRed = (myBYTE)r;
				co = strtok(NULL,",");
				if (co != NULL)
				{
					int g = atoi(co);
					rgb.rgbGreen = (myBYTE)g;
					co = strtok(NULL,",");
					if (co != NULL)
					{
						int b = atoi(co);
						rgb.rgbBlue = (myBYTE)b;
					}
				}
			}	
		}
	}
	else if (color.find("#") != string::npos)
	{
again:	string strR,strG,strB;
		if (color.size() < 6)
		{
			strR = color.substr(1,1);
			strR.append(strR);
			strG = color.substr(2,1);
			strG.append(strG);
			strB = color.substr(3,1);
			strB.append(strB);
		}else
		{
			strR = color.substr(1,2);
			strG = color.substr(3,2);
			strB = color.substr(5,2);
		}
		int r = m_atoi(strR.c_str(),16);
		int g = m_atoi(strG.c_str(),16);
		int b = m_atoi(strB.c_str(),16);
		rgb.rgbRed = (myBYTE)r;
		rgb.rgbGreen = (myBYTE)g;
		rgb.rgbBlue = (myBYTE)b;

	}
	else
	{
		int i;
		for (i = 0;i<134;i++)
		{
			if (color.compare(colorName[i]) == 0)
			{
				break;
			}
		}
		if (i < 134)
		{
			color = colorCode[i];
			goto again;
		}
	}
	return rgb;
}

myBYTE CStyleSheetParser::ReturnFrontSizeFromString(string str)
{
	myBYTE size;
	if (str.compare("xx-large") == 0)
	{
		size = 32;
	}
	else if (str.compare("x-large") == 0)
	{
		size = 24;
	}
	else if (str.compare("large") == 0)
	{
		size = 18;
	}
	else if (str.compare("medium") == 0)
	{
		size = 16;
	}
	else if (str.compare("small") == 0)
	{
		size = 13;
	}
	else if (str.compare("x-small") == 0)
	{
		size = 10;

	}
	else if (str.compare("xx-small") == 0)
	{
		size = 9;
	}
	//像素
	else if (str.find("px") != string::npos)
	{
		size = ReturnLevelFromPX(str);
	}
	//百分比
	else if (str.find("%") != string::npos)
	{
		size = ReturnLevelFromPercent(atoi(str.c_str()));
	}
	//长度单位
	else if (str.find("em") != string::npos)
	{
		size = ReturnLevelFromEm(str);
	}
	else if (str.find("pt") != string::npos)
	{
		size_t pos = str.find("pt");
		str.replace(pos,2,"px");
		size = ReturnLevelFromPX(str)*96/72;
	}
	else
		size = 4;
	return size;
}

myBYTE CStyleSheetParser::ReturnLevelFromPX(string strpx)
{
	size_t pos = strpx.find("px");
	if (pos != string::npos)
	{
		int level = atoi((strpx.substr(0,pos)).c_str());
		return level;
	}
	else
	{
		return DEFAULTFONTSIZE;
	}
}

myBYTE CStyleSheetParser::ReturnLevelFromPercent(int percent)
{
	if (percent < 0 || percent > 100)
	{
		return 4;
	}
	if (percent > 80)
	{
		return 1;
	}
	else if (percent > 70)
	{
		return 2;
	}
	else if (percent > 60)
	{
		return 3;
	}
	else if (percent > 50)
	{
		return 4;
	}
	else if (percent > 40)
	{
		return 5;
	}
	else if (percent > 30)
	{
		return 6;
	}
	else
		return 7;
}

short CStyleSheetParser::ReturnFrontWeigthFromString(string str)
{
	if (str == "normal")
	{
		return 4;
	}
	else if (str == "bold")
	{
		return 7;
	}
	else if (str == "bolder")
	{
		return 6;
	}
	else if (str == "lighter")
	{
		return 3;
	}
	else
	{
		int size = atoi(str.c_str());
		short weigth = size / 100;
		return weigth;
	}
}

myBYTE CStyleSheetParser::ReturnLevelFromEm(string strem)
{
	size_t pos = strem.find("em");
	if (pos != string::npos)
	{
		string newstrem = GetPxFormEm(strem);
		return ReturnLevelFromPX(newstrem);
	}
	else
		return DEFAULTFONTSIZE;
	
}

void CStyleSheetParser::ParserInline(void)
{
	string attrvalue;//属性值
	string attrname;//属性名

	Selector select;
	select.selectorName = "inline";
	int startPosition(0);
	myBYTE state = ATTRIBUTE_NAME;
	for (int i = -1;i < m_cssLength;)
	{
		switch(state)
		{	
		case ATTRIBUTE_NAME:
			switch(m_cssData[++i])
			{
			case 0x0008:
			case 0x0009:
			case 0x000A:
			case 0x000B:
			case 0x000C:
			case 0x000D:
			case ' ':	
				break;
			case ':':
				attrname.append((char*)(m_cssData + startPosition),i - startPosition);
				state = ATTRIBUTE_VAULE;
				startPosition = i + 1;
				break;

			}
			break;
		case ATTRIBUTE_VAULE:
			switch(m_cssData[++i])
			{
			case 0x0008:
			case 0x0009:
			case 0x000A:
			case 0x000B:
			case 0x000C:
			case 0x000D:
			case ' ':	
				break;
			case ';':
				attrvalue.append((char*)(m_cssData + startPosition),i - startPosition);
				m_ToLower(attrvalue);
				m_ToLower(attrname);
				select.attrs.insert(std::pair<string,string>(attrname,attrvalue));
				attrname.erase();
				attrvalue.erase();
				state = ATTRIBUTE_NAME;
				startPosition = i + 1;
				break;
			}
			break;

		}
	}
	if (attrvalue.empty())
	{
		attrvalue.append((char*)(m_cssData + startPosition),m_cssLength - startPosition);
		m_ToLower(attrvalue);
		m_ToLower(attrname);
		select.attrs.insert(std::pair<string,string>(attrname,attrvalue));
		attrname.erase();
		attrvalue.erase();
	}
	Process(select);
}

int CStyleSheetParser::ProcessMargin(StyleSheet& css,string value)
{
	char* str = (char*)value.c_str();
	
	string mar1;
	string mar2;
	string mar3;
	string mar4;
	int margin1,margin2,margin3,margin4;
	int number(0);
	char* ptr = strtok(str," ");
	if(ptr != NULL)
	{
		mar1.assign(ptr);
		number++;
		ptr = strtok(NULL," ");
		if(ptr != NULL)
		{
			mar2.assign(ptr);
			number++;
			ptr = strtok(NULL," ");
			if(ptr != NULL)
			{
				mar3.assign(ptr);
				number++;
				ptr = strtok(NULL," ");
				if(ptr != NULL)
				{
					mar4.assign(ptr);
					number++;
					
				}
			}
		}
	}
	css.ismargin = true;
	if(mar1.find("em") != string::npos)
	{
		mar1 = GetPxFormEm(mar1);
	}
	margin1 = atoi(mar1.c_str());
	if(mar2.find("em") != string::npos)
	{
		mar2 = GetPxFormEm(mar2);
	}
	margin2 = atoi(mar2.c_str());
	if(mar3.find("em") != string::npos)
	{
		mar3 = GetPxFormEm(mar3);
	}
	margin3 = atoi(mar3.c_str());
	if(mar4.find("em") != string::npos)
	{
		mar4 = GetPxFormEm(mar4);
	}
	margin4 = atoi(mar4.c_str());
	switch(number)
	{
	case 1:
		css.margin_top = css.margin_right = css.margin_left = css.margin_bottom = margin1;
		break;
	case 2:
		css.margin_top = css.margin_bottom = margin1;
		css.margin_left = css.margin_right = margin2;
		break;
	case 3:
		css.margin_top = margin1;
		css.margin_left = css.margin_right = margin2;
		css.margin_bottom = margin3;
		break;
	case 4:
		css.margin_top = margin1;
		css.margin_right = margin2;
		css.margin_bottom = margin3;
		css.margin_left = margin4;
		break;
	default:
		css.ismargin = false;
	}
	return 0;
}

string CStyleSheetParser::GetPxFormEm(string em)
{
	string strpx;
	size_t pos = em.find("em");
	if (pos != string::npos)
	{
		double dem = atof((em.substr(0,pos)).c_str());
		int px = dem * 16;//1em=16px
		char szpx[4];
		//itoa(px,szpx,10);
		sprintf(szpx,"%d",px);
		strpx.assign(szpx);
		strpx.append("px");
	}
	else
		strpx.assign("");
	return strpx;
}

std::string CStyleSheetParser::DeleteNote( string& str )
{
	
again:	string::size_type sn = str.find("/*");
	if (sn != string::npos)
	{
		string::size_type en = str.find("*/");
		if (en != string::npos )
		{
			if (en > sn)
			{
				str.erase(sn,en+2);
				goto again;
			}
		}
	}
	return str;
}
