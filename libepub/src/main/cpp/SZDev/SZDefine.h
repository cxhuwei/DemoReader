// SZDefine.h: interface for the BWImageScalingAlgorithm class.
//
//////////////////////////////////////////////////////////////////////
/*
* Copyright (c) 2011,  ±±Ÿ©³¬ÐÇÑÐ·¢²¿ 
* All rights reserved.
*
* ÎÄŒþÃû³Æ£ºSZDefine.h
* ÎÄŒþ±êÊ¶£ºSZDEFINE_H
* Õª Òª£º   Ô€¶šÒåÒ»Ð©ÓÃµœµÄÊýŸ�?
*
* µ±Ç°°æ±Ÿ£º1.0
* ×÷ Õß£º   ÉÛÕñ     shaozhen1@163.com
* Íê³ÉÈÕÆÚ£º2011Äê09ÔÂ02ÈÕ
*/
#ifndef SZDEFINE_H
#define SZDEFINE_H
#include <string>
#include <vector>
#include <map>
#include <stack>
using namespace std;

#define DT_EX_LEFT							0x00000001
#define DT_EX_CENTER						0x00000002
#define DT_EX_RIGHT							0x00000004
#define DT_EX_TOP							0x00000008
#define DT_EX_VCENTER						0x00000010
#define DT_EX_BOTTOM						0x00000020
#define DT_EX_VERTICAL						0x00000040
#define DT_EX_RTOLREADING					0x00000080
#define DT_EX_UNDERLINE						0x00000100
#define DT_EX_BACKCOLOR 					0x00000200
#define DT_EX_BORDER						0x00000400

// enum HTMLNodeType
// {
// 		NODE_UNKNOWN = 0,
// 		NODE_START_TAG,//¿ªÊŒ±êÇ©
// 		NODE_CLOSE_TAG,//œáÊø±êÇ©
// 		NODE_FULL_TAG,
// 		NODE_CONTENT,//ÄÚÈÝ
// 		// 	NODE_COMMENT,//×¢ÊÍ±êÇ©
// 		// 	NODE_SCRIPT,//script±êÇ©
// };
// enum HTMLTagType
// {
// 		TAG_UNKNOWN = 0,
// 		TAG_A,TAG_DIV,TAG_FONT,TAG_IMG,TAG_P,TAG_SPAN,TAG_BR,TAG_B,TAG_I,TAG_HR,
// 		TAG_COMMENT,TAG_SCRIPT,TAG_STYLE,
// 		TAG_H1,TAG_H2,TAG_H3,TAG_H4,TAG_H5,TAG_H6
// };
// enum Encodings
// {
// 		UNKNOWN = 0,
// 		GBK,
// 		GB2312,
// 		GB18030,
// 		UTF8,
// 		BIG5,	
// };
#define  MAX_HTML_TAG_LENGTH 15
typedef unsigned char  myBYTE;
typedef unsigned short myWORD;
typedef unsigned int   myDWORD;

#if defined (_WIN32) || defined(WIN32)
typedef __int64  my64long;
typedef unsigned __int64  myU64long;
#else /* Linux */
typedef long long  my64long;
typedef unsigned long long  myU64long;
#endif

#define MAKEMYWORD(a, b)      ((myWORD)(((myBYTE)(a)) | ((myWORD)((myBYTE)(b))) << 8))
#define MAKEMYDWORD(a, b)     ((myDWORD)(((myWORD)(a)) | ((myDWORD)((myWORD)(b))) << 16))
#define LOMYWORD(l)           ((myWORD)(l))
#define HIMYWORD(l)           ((myWORD)(((myDWORD)(l) >> 16) & 0xFFFF))
#define LOMYBYTE(w)           ((myBYTE)(w))
#define HIMYBYTE(w)           ((myBYTE)(((myWORD)(w) >> 8) & 0xFF))

//////////////CSS////////////////////////////////////////////////////////

// typedef struct tagmyRGBQUAD 
// {
// 	tagmyRGBQUAD()
// 	{
// 		rgbBlue = rgbRed = rgbGreen = 0;
// 	}
// 	myBYTE    rgbBlue;
// 	myBYTE    rgbGreen;
// 	myBYTE    rgbRed;
// 	myBYTE    rgbReserved;
// }myRGBQUAD,* myLPRGBQUAD;


// const myBYTE SELECTOR_TYPE_TAG = 0;
// const myBYTE SELECTOR_TYPE_CLASS = 1;
// const myBYTE SELECTOR_TYPE_ID = 2;
// //////////////////////////////////////////////////////////////////////////
// class ContentStyle
// {//ÎÄ±ŸÑùÊœ
// public:
// 	ContentStyle()
// 	{
// 		frontSize = 4;
// 		Bold = 0;
// 		isItalic = isUnderline = isP = false;
// 	}
// 	string frontfamily;//×ÖÌå
// 	myBYTE frontSize;//×ÖÌåŽóÐ¡  18ÏñËØÎªÖÐŒä
// 	myRGBQUAD frontColor;//×ÖÌåÑÕÉ«0xAARRGGBB±íÊŸ
// 	myRGBQUAD backGroundColor;//±³Ÿ°ÑÕÉ«
// 	short Bold;//×ÖÌåŒÓŽÖ 0-ÎªÕý³£ÏÔÊŸ 1-9ÎªŒÓŽÖŒ¶±ð
// 	bool isItalic;//×ÖÌåÊÇ·ñÐ±Ìå
// 	bool isUnderline;//ÊÇ·ñÓÐÏÂ»®Ïß
// 	bool isP;//ÊÇ·ñÊÇp±êÇ©£¬Ä¿µÄÎªÁË·Ö¶ÎÂä
// };
// class StyleSheet : public ContentStyle
// {
// public:
// 	StyleSheet operator = (const StyleSheet& object)
// 	{
// 		StyleSheet thisobject;
// 		thisobject.cssType = object.cssType;
// 		thisobject.frontfamily = object.frontfamily;
// 		if (object.frontSize != 4)
// 		{
// 			thisobject.frontSize = object.frontSize;
// 		}
// 		if (Bold != 0)
// 		{
// 			thisobject.Bold = object.Bold;
// 		}
// 		thisobject.frontColor = object.frontColor;
// 		thisobject.backGroundColor = object.backGroundColor;
// 		if (object.isItalic)
// 		{
// 			thisobject.isItalic = true;
// 		}
// 		if (object.isUnderline)
// 		{
// 			thisobject.isUnderline = true;
// 		}
// 		if (object.isP)
// 		{
// 			thisobject.isP = true;
// 		}
// 		return thisobject;
// 	}
// 	myBYTE cssType;
// };
// //////////////////////////////////////////////////////////////////////////
// class Content
// {
// public:
// 	wchar_t		*before;
// 	wchar_t		*text;//ÎÄ±ŸÄÚÈÝ
// 	wchar_t		*after;
// 	StyleSheet   textStyle;//ÎÄ±ŸÑùÊœ
// };


// typedef struct _Record
// {
// 	string file;
// 	int contentNumber;//ÎÄ±Ÿ±êÇ©µÄÐòºÅ
// 	int offer;//
// }Record;

struct note
{
	int noteId;
	int fileId;
	int s_contentID;
	int s_offset;
	int e_contentID;
	int e_offset;
	int type;
	int colorIndex;
};
typedef vector<note> Vt_notes;
//typedef map<int,note> Mt_notes;

struct curPageNote
{
	int fileID;//����html�ļ�id
	int curPageID;//�ڵ�ǰҳ �� id
	int curBookId;//���������  id 
	int type;
	int s_contentID;
	int s_offset;
	int e_contentID;
	int e_offset;
	int status;//0-��������?1-����2-�޸� 3-ɾ��
	int colorIndex;
};
typedef vector<curPageNote> Vt_curPageNotes;
struct curPageNotes
{
	int maxID;

	Vt_curPageNotes Notes;
};

enum ColorType
{
	AutoChange = 0,
	NoChange
};

//ҳ�ڶ�ý������
enum MultimediaType
{
	MT_Image = 1,//ͼ��
	MT_Link = 2,//������
	MT_Video = 3//��Ƶ
};

struct MultimediaPage
{
	MultimediaPage()
	{
		this->lefttop_x = 0;
		this->lefttop_y = 0;
		this->rightbottom_x = 0;
		this->rightbottom_y = 0;
		this->type = MT_Link;
// 		this->dataSize = 0;
// 		this->data = NULL;
	}
// 	MultimediaPage(const MultimediaPage& object)
// 	{
// 		this->lefttop_x = object.lefttop_x;
// 		this->lefttop_y = object.lefttop_y;
// 		this->rightbottom_x = object.rightbottom_x;
// 		this->rightbottom_y = object.rightbottom_y;
// 		this->type = object.type;
// 		this->dataSize = object.dataSize;
// 		if (object.dataSize > 0)
// 		{
// 			this->data = new myBYTE[object.dataSize];
// 			memcpy(this->data,object.data,object.dataSize);
// 		}
// 	}
// 	MultimediaPage operator = (const MultimediaPage& object)
// 	{
// 		this->lefttop_x = object.lefttop_x;
// 		this->lefttop_y = object.lefttop_y;
// 		this->rightbottom_x = object.rightbottom_x;
// 		this->rightbottom_y = object.rightbottom_y;
// 		this->type = object.type;
// 		this->dataSize = object.dataSize;
// 		if (object.dataSize > 0)
// 		{
// 			this->data = new myBYTE[object.dataSize];
// 			memcpy(this->data,object.data,object.dataSize);
// 		}
// 		return *this;
// 	}
	int lefttop_x;
	int lefttop_y;
	int rightbottom_x;
	int rightbottom_y;

	MultimediaType type;//��ý������
// 	myBYTE* data;
// 	long	dataSize;
	string	filePath;//
	string	fileType;//�ļ�����

};
typedef vector<MultimediaPage> Vt_curPageMultiMedia;

struct interlink
{
	int lefttop_x;
	int lefttop_y;
	int rightbottom_x;
	int rightbottom_y;
	char url[256];
};
typedef vector<interlink> Vt_curPageInterLinks;
#endif //SZDEFINE_H
