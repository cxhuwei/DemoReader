
#ifndef MYFDEFINE_H
#define MYFDEFINE_H
#include "../../SZDev/SZDefine.h"
#include <string>
#include <vector>
#include <map>
using namespace std;


//////////////////////////////////code///////////////////////////////////
//#if defined (_WIN32) || defined(WIN32)
typedef wchar_t  myWcharT;

//////////////////////////////////////////////////////////////////////////
enum HTMLNodeType
{
		NODE_UNKNOWN = 0,
		NODE_START_TAG,//开始标签
		NODE_CLOSE_TAG,//结束标签
		NODE_FULL_TAG,
		NODE_CONTENT,//内容
		// 	NODE_COMMENT,//注释标签
		// 	NODE_SCRIPT,//script标签
};
enum HTMLTagType
{
		TAG_UNKNOWN = 0,
		TAG_A,TAG_DIV,TAG_FONT,TAG_IMG,TAG_P,TAG_SPAN,TAG_BR,TAG_B,TAG_I,TAG_HR,
		TAG_COMMENT,TAG_SCRIPT,TAG_STYLE,
		TAG_H1,TAG_H2,TAG_H3,TAG_H4,TAG_H5,TAG_H6,
		TAG_LI,TAG_UL,TAG_SUP
};
enum Encodings
{
		UNKNOWN = 0,
		GBK,
		GB2312,
		GB18030,
		UTF8,
		BIG5,	
};
#define  MAX_HTML_TAG_LENGTH 15

//////////////CSS////////////////////////////////////////////////////////
enum ALIGN
{
	CENTER,
	LEFT,
	RIGHT
};
typedef struct tagmyfRGBQUAD 
{
	myBYTE    rgbBlue;
	myBYTE    rgbGreen;
	myBYTE    rgbRed;
	myBYTE    rgbReserved;
}myfRGBQUAD,* myfLPRGBQUAD;


const myBYTE SELECTOR_TYPE_TAG = 0;
const myBYTE SELECTOR_TYPE_CLASS = 1;
const myBYTE SELECTOR_TYPE_ID = 2;
//////////////////////////////////////////////////////////////////////////
#define DEFAULTFONTSIZE 16

enum borderStyle{none,//定义无边框。
				hidden,//与 "none" 相同。不过应用于表时除外，对于表，hidden 用于解决边框冲突。
				dotted,//定义点状边框。在大多数浏览器中呈现为实线。
				dashed,//定义虚线。在大多数浏览器中呈现为实线。
				solid};//定义实线。
struct t_Border
{
	int border_width;
	borderStyle border_style;
	myfRGBQUAD border_color;
};

class ContentStyle
{//文本样式
public:
	ContentStyle()
	{
		fontSize = DEFAULTFONTSIZE;
		Bold = 0;
		isItalic = isUnderline = false;
		textAlign = LEFT;
		UnderlineIsVaild = false;
		textAlignIsVaild = false;
		isFontColor = false;
		isBackGroundColor = false;
	}
	//string fontfamily;//字体
	myBYTE fontSize;//字体大小  16像素为中间

	myfRGBQUAD fontColor;//字体颜色0xAARRGGBB表示
	bool isFontColor;//是否有字体颜色样式

	myfRGBQUAD backGroundColor;//背景颜色
	bool isBackGroundColor;//是否有背景颜色样式

	short Bold;//字体加粗 0-为正常显示 1-9为加粗级别
	bool isItalic;//字体是否斜体

	bool isUnderline;//是否有下划线
	bool UnderlineIsVaild;//下划线是否有用

	ALIGN textAlign;//文本水平排列方式
	bool textAlignIsVaild;//对齐是否有效
};
class StyleSheet : public ContentStyle
{
public:

	StyleSheet operator = (const StyleSheet& object)
	{
		
		this->cssType = object.cssType;
		//this->fontfamily = object.fontfamily;
		if (object.fontSize != 4)
		{
			this->fontSize = object.fontSize;
		}
		if (Bold != 0)
		{
			this->Bold = object.Bold;
		}
		this->isFontColor = object.isFontColor;
		this->isBackGroundColor = object.isBackGroundColor;
		this->fontColor = object.fontColor;
		this->backGroundColor = object.backGroundColor;
		if (object.isItalic)
		{
			this->isItalic = true;
		}
		if (object.UnderlineIsVaild)
		{
			this->UnderlineIsVaild = true;
			this->UnderlineIsVaild = object.isUnderline;
		}
		if (object.textAlignIsVaild)
		{
			this->textAlignIsVaild = true;
			this->textAlign = object.textAlign;
		}
		//this->isUnderline = object.isUnderline;
		//this->textAlign = object.textAlign;
		this->ismargin = object.ismargin;
		this->margin_top = object.margin_top;
		this->margin_bottom = object.margin_bottom;
		this->margin_left = object.margin_left;
		this->margin_right = object.margin_right;
		return *this;
	}
	myBYTE cssType;
	bool ismargin;
	myBYTE margin_top;
	myBYTE margin_left;
	myBYTE margin_right;
	myBYTE margin_bottom;

// 	t_Border borderTop;
// 	t_Border borderLeft;
// 	t_Border borderRight;
// 	t_Border borderBottom;
};

class ImgStyle
{
public:
	ImgStyle()
	{

	}

	int imgWidth;
	int imgHeight;
	int posX;
	int posY;
};
//////////////////////////////////////////////////////////////////////////
enum ContentType
{
	CT_Text = 0x0001,
	CT_Image = 0x0010,
	CT_ImagePage = 0x0100,
	CT_Link = 0x1000,//超链接
	CT_SUP = 0x10000//上标
};

class Content
{
public:
	Content()
	{
		contentType = CT_Text;
		isHaveStyle = false;
		isP_start = isP_end = false;
		titleType = 0;
		before = NULL;
		text = NULL;
		after = NULL;
		txt_SrcPtext = NULL;
	}
	myWcharT		*before;
	myWcharT		*text;//文本内容
	myWcharT		*after;
	StyleSheet   textStyle;//文本样式
	string		filepath;//图像文件路径
	bool         isHaveStyle;
	//bool		isFirst;
	int		  contentType;//类型
	int			titleType;//标题类型 1-7对应h1-h7 0不是标题
	bool isP_start;//是否是p标签，目的为了段落开始的空格
	bool isP_end;//目的为了在后面增加换行符
	myWcharT		*txt_SrcPtext;//txt文件的原始指针，delete用
  //	int			encodetype;
};


//////////////////////////////////////////////////////////////////////////
typedef struct _Record
{
	string file;
	int contentNumber;//文本标签的序号
	int offer;//
}Record;

struct bookinfo
{
	bookinfo()
	{
		title = author = subject = description = publisher = date = language = NULL;
		cover = NULL;
		coverDatalength = 0;
	}
	myWcharT* title;
	myWcharT* author;//m_creator;
	myWcharT* subject;
	myWcharT* description;
	myWcharT* publisher;
	myWcharT* date;
	myWcharT* language;
	myBYTE* cover;
	int coverDatalength;
};

struct pageNumberRange
{
	int startNumber;
	int endNumber;
};
struct chapterInfor
{
	int order;
	wchar_t* chapterName;
	pageNumberRange NumberRange;
};


struct  wordinfo  
{
	int lineIndex;
	int fileid;
	unsigned int WordUnicod;
	int contentID;
	int offset;
	int x;
	int y;
	int width;
	int height;
	//扩展
	int			titleType;//标题类型 1-7对应h1-h7 0不是标题
    bool isRealChar;
	StyleSheet   textStyle;//文本样式
};
typedef vector<wordinfo> Vt_wordinfo;

struct lineinfo
{ 
	int x;
	int y;
	int Width;
	int Height;
	int RealwordNum;	
	Vt_wordinfo words;
};
typedef vector<lineinfo> Vt_lineinfo;

struct imageInfo
{
	int x;
	int y;
	int width;
	int height;
	string filePath;
};
struct pageWordinfo
{ 
	int    fileID;
	int	contentID;
	Vt_lineinfo lines;
	vector<imageInfo> images;
};


struct myBookRecord
{
	int vfileID;
	int contentNumber;//文本标签的序号
	int offer;

	bool operator>(myBookRecord& obj)
	{
		if (vfileID > obj.vfileID)
		{
			return true;
		}
		else if (vfileID < obj.vfileID)
			return false;

		if (contentNumber>obj.contentNumber)
		{
			return true;
		}
		else if (contentNumber<obj.contentNumber)
		{
			return false;
		}

		if (offer>obj.offer)
		{
			return true;
		}	
		return false;
	}

	bool operator<(myBookRecord& obj)
	{
		if (vfileID < obj.vfileID)
			return true;
		else if(vfileID > obj.vfileID)
			return false;

		if (contentNumber<obj.contentNumber)
		{
			return true;
		}
		else if (contentNumber>obj.contentNumber)
		{
			return false;
		}

		if (offer<obj.offer - 1)
		{
			return true;
		}	
		return false;
	}
	bool operator == (myBookRecord& obj)
	{
		if (contentNumber == obj.contentNumber && vfileID == obj.vfileID && offer == obj.offer)
		{
			return true;
		}
		else
			return false;
	}
	bool operator != (myBookRecord& obj)
	{
		if (contentNumber != obj.contentNumber || vfileID != obj.vfileID || offer != obj.offer)
		{
			return true;
		}
		else
			return false;
	}
};
struct t_PageOutData
{
	myBookRecord nextPageRecord;//下一页第一个字的信息
	pageWordinfo pwi;//当前页文字信息
	Vt_curPageMultiMedia pmm;//当前页媒体数据注信息
	curPageNotes PageNotes;//当前页到标注信息
};
#endif //SZDEFINE_H
