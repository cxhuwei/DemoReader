#ifndef _ZLFILE_H
#define _ZLFILE_H

#include "../SZDev/SZDefine.h"
#include "unzip.h"
#include "HtmlParser.h"
#include "StyleSheetParser.h"


#define EXTEN_LENGTH 50
#define SHORTNAME_LENGTH 256

#define OPENZIPFILE_OK			0 //打开文件成功
#define OPENZIPFILE_LASTOPEN	1 //文件上次成功打开过
#define OPENZIPFILE_NOT_EXIST  -2 //压缩包里不存在这个文件
#define OPENZIPFILE_DATA_ERROR -1 //解压缩数据失败

class CZLFile
{
public:
	CZLFile();
	CZLFile(unzFile* unzfile);
	~CZLFile(void);
private:
	char m_extension[EXTEN_LENGTH];
	char m_shortName[SHORTNAME_LENGTH];
	char* m_filePath;
	char* m_tmpFilePath;
	unzFile* m_unzFile;
	myBYTE* m_DataFile;
	int m_DataLength;
	char m_anchorName[SHORTNAME_LENGTH];
	bool m_isLastOpen;//之前是否打开过

	Record m_tAnchor;//记录锚点信息

	

public:
	bool LastOpen();
	int OpenFile(const char* file,bool isCleanHtmlData = true,bool isCleanCssData=true);
	char* GetExtension(void);
	int PutFilePath(const char* filepath);
	// 返回文件的数据
	myBYTE* GetFileData(void);
	int GetDataLength(void);
	// 解析该文件的html结构
	int Parser(void);
	void ClearHtmlData();
	Record* GetAnchor();
	map<string,int> GetAnchorContentNumber();

public:
	vector<Content> m_vContent;//最终文本
	bool m_isAnchor;// 标记该文件是否存在锚点
	map<string,int> m_mAnchorNameContentNumber;
	VtHTMLNode m_vHtmlNode;
	map<string,StyleSheet> m_MCSS;
	myWcharT* m_HtmlTitle;
private:
	bool m_isHaveCSS;
	string m_cssFile;
	vector<string> m_vCssFiles;
	
public:
	bool IsHaveCSS(void);
	vector<string> ReturnCSSFilepath(void);
	int ParserCSS(void);
	int ProcessHtmlCSS(void);
private:
	string m_oebpsDir;
public:
	void PutOebpsDir(std::string oebpsdir);
	void GetFrontStyleFormTag(HTMLTagType tagtype, Content* content);
	// 把解析html对象中的内部样式放到zlfile的样式表中。
	void PutCssIntoFileCss(CStyleSheetParser& csspar);
private:
	// 当前文件所在目录
	string m_curDir;
public:
	string GetCurDir(void);
	string MergePath(string path, string root);
	void ClearCSSData(void);
	string GetCurFilePath();
};
#endif