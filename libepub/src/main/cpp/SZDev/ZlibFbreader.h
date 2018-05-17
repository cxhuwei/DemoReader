#pragma once
#include "unzip.h"
#include "Book.h"
#include "MAlgorithm.h"
//enum EncodingType {OPENFAIL=-1,READERR=0,ANSI=1,UTF16_LE,UTF16_BE,UTF32_LE,UTF32_BE,UTF_8};

#define FILETYPE_EPUB 0
#define FILETYPE_TXT  1
class CZEpubReader
{
public:
	CZEpubReader(void);
	~CZEpubReader(void);
	int OpenFile(char* filepath);
	int Close();
	//int ReadFileAndParser(const char* file);
	int putRootFile(const char* rootfile);	
	void PutNcxFile(string& ncx);
	CBook* m_book;//打开的这本书
	unzFile* m_epubFile;// 解压缩对象
	void SetEpubOpp(unzFile* epubfile);
private:
	int OpenMETA_INF();
	int OpenRootFile();
	int OpenOebpsFile();
	int CreateRootFile(int length);

	int AutoTakeChapterinfo(EncodingType encode,Content* cont,char* filepath,int contentID,MSI& msi);
public:
	int m_curFileType;//0-epub 1-txt
private:
	
	char* m_rootFile;//
	char* ncxFile;
	
	string oebpsDir;
	bool isClear;

	int  m_chapterorder;
private:
	bool META_INF_manifest;//meta-inf文件夹下面是否存在manifest.xml文件
	bool META_INF_metadata;
	bool META_INF_signatures;
	bool META_INF_encryption;
	bool META_INF_rights;//meta-inf文件夹下面是否存在rights.xml文件

public:
	int OpenTxt(char* filepath);
	void FreeFile(void);
	// 通过order读取文件
	int ReadFileByOrder(int order);
	//通过filepath读取文件
	int ReadFileByFilePath(string file);
	// 打开媒体文件（图像）
	int ReadMediaFileByFilePath(string file,myBYTE* data,int& datalength);
	int OpenFileByFilePath(string file);
};
