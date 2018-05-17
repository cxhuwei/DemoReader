
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
		NODE_START_TAG,//��ʼ��ǩ
		NODE_CLOSE_TAG,//������ǩ
		NODE_FULL_TAG,
		NODE_CONTENT,//����
		// 	NODE_COMMENT,//ע�ͱ�ǩ
		// 	NODE_SCRIPT,//script��ǩ
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

enum borderStyle{none,//�����ޱ߿�
				hidden,//�� "none" ��ͬ������Ӧ���ڱ�ʱ���⣬���ڱ�hidden ���ڽ���߿��ͻ��
				dotted,//�����״�߿��ڴ����������г���Ϊʵ�ߡ�
				dashed,//�������ߡ��ڴ����������г���Ϊʵ�ߡ�
				solid};//����ʵ�ߡ�
struct t_Border
{
	int border_width;
	borderStyle border_style;
	myfRGBQUAD border_color;
};

class ContentStyle
{//�ı���ʽ
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
	//string fontfamily;//����
	myBYTE fontSize;//�����С  16����Ϊ�м�

	myfRGBQUAD fontColor;//������ɫ0xAARRGGBB��ʾ
	bool isFontColor;//�Ƿ���������ɫ��ʽ

	myfRGBQUAD backGroundColor;//������ɫ
	bool isBackGroundColor;//�Ƿ��б�����ɫ��ʽ

	short Bold;//����Ӵ� 0-Ϊ������ʾ 1-9Ϊ�Ӵּ���
	bool isItalic;//�����Ƿ�б��

	bool isUnderline;//�Ƿ����»���
	bool UnderlineIsVaild;//�»����Ƿ�����

	ALIGN textAlign;//�ı�ˮƽ���з�ʽ
	bool textAlignIsVaild;//�����Ƿ���Ч
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
	CT_Link = 0x1000,//������
	CT_SUP = 0x10000//�ϱ�
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
	myWcharT		*text;//�ı�����
	myWcharT		*after;
	StyleSheet   textStyle;//�ı���ʽ
	string		filepath;//ͼ���ļ�·��
	bool         isHaveStyle;
	//bool		isFirst;
	int		  contentType;//����
	int			titleType;//�������� 1-7��Ӧh1-h7 0���Ǳ���
	bool isP_start;//�Ƿ���p��ǩ��Ŀ��Ϊ�˶��俪ʼ�Ŀո�
	bool isP_end;//Ŀ��Ϊ���ں������ӻ��з�
	myWcharT		*txt_SrcPtext;//txt�ļ���ԭʼָ�룬delete��
  //	int			encodetype;
};


//////////////////////////////////////////////////////////////////////////
typedef struct _Record
{
	string file;
	int contentNumber;//�ı���ǩ�����
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
	//��չ
	int			titleType;//�������� 1-7��Ӧh1-h7 0���Ǳ���
    bool isRealChar;
	StyleSheet   textStyle;//�ı���ʽ
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
	int contentNumber;//�ı���ǩ�����
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
	myBookRecord nextPageRecord;//��һҳ��һ���ֵ���Ϣ
	pageWordinfo pwi;//��ǰҳ������Ϣ
	Vt_curPageMultiMedia pmm;//��ǰҳý������ע��Ϣ
	curPageNotes PageNotes;//��ǰҳ����ע��Ϣ
};
#endif //SZDEFINE_H
