#ifndef _ZLFILE_H
#define _ZLFILE_H

#include "../SZDev/SZDefine.h"
#include "unzip.h"
#include "HtmlParser.h"
#include "StyleSheetParser.h"


#define EXTEN_LENGTH 50
#define SHORTNAME_LENGTH 256

#define OPENZIPFILE_OK			0 //���ļ��ɹ�
#define OPENZIPFILE_LASTOPEN	1 //�ļ��ϴγɹ��򿪹�
#define OPENZIPFILE_NOT_EXIST  -2 //ѹ�����ﲻ��������ļ�
#define OPENZIPFILE_DATA_ERROR -1 //��ѹ������ʧ��

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
	bool m_isLastOpen;//֮ǰ�Ƿ�򿪹�

	Record m_tAnchor;//��¼ê����Ϣ

	

public:
	bool LastOpen();
	int OpenFile(const char* file,bool isCleanHtmlData = true,bool isCleanCssData=true);
	char* GetExtension(void);
	int PutFilePath(const char* filepath);
	// �����ļ�������
	myBYTE* GetFileData(void);
	int GetDataLength(void);
	// �������ļ���html�ṹ
	int Parser(void);
	void ClearHtmlData();
	Record* GetAnchor();
	map<string,int> GetAnchorContentNumber();

public:
	vector<Content> m_vContent;//�����ı�
	bool m_isAnchor;// ��Ǹ��ļ��Ƿ����ê��
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
	// �ѽ���html�����е��ڲ���ʽ�ŵ�zlfile����ʽ���С�
	void PutCssIntoFileCss(CStyleSheetParser& csspar);
private:
	// ��ǰ�ļ�����Ŀ¼
	string m_curDir;
public:
	string GetCurDir(void);
	string MergePath(string path, string root);
	void ClearCSSData(void);
	string GetCurFilePath();
};
#endif