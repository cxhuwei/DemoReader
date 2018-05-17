#include "StdAfx.h"
#include "ZlibFbreader.h"
#include "zlib.h"
#include <string>
#include "XMLParser.h"
#include "ZLFile.h"
#include "iconv.h"
#include "regex.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#include <android/log.h>
#define LOG_TAG "SZEbookReader"
#define LOGI(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

extern "C"
{
	int do_extract_currentfile(unzFile uf,const int* popt_extract_without_path,int* popt_overwrite,char* root);
	int ReadOneFileToMem(unzFile uf,char* filename,myBYTE** buffer,int* length);
}

CZEpubReader::CZEpubReader(void)
:m_book(NULL)
{
	m_epubFile = NULL;
	m_rootFile = NULL;
	ncxFile = NULL;
	META_INF_encryption = META_INF_manifest = META_INF_metadata = META_INF_rights = META_INF_signatures = true;
	isClear = true;
}



CZEpubReader::~CZEpubReader(void)
{

	Close();
	
}

int CZEpubReader::OpenFile(char* filepath)
{	
	char* ptr = strrchr(filepath,'.');
	if (ptr)
	{
		ptr++;
		if (myStrCaseCmp(ptr,"txt") == 0)
		{
			m_curFileType = FILETYPE_TXT;
			return OpenTxt(filepath);
		}
	}
	m_curFileType = FILETYPE_EPUB;
    m_epubFile = new unzFile();
    if (m_epubFile == NULL) {
        return -1;
    }
	*m_epubFile = unzOpen(filepath);

 	if (*m_epubFile)
 	{
		m_book = new CBook(m_epubFile);
		//1.step
		//��ѹepub�ļ�
		if (0 != OpenMETA_INF())
		{
			return -2;
		}

		//	2.step ����������Ҫ�ļ�
		if (m_rootFile != NULL)
		{
			//openrootfile
			if (0 != OpenRootFile())
			{
				return -3;
			}
		}
		if (ncxFile != NULL)
		{
			if (0 != OpenOebpsFile())
			{
				return 0;
			}

		}
	}
 	else
 		return -1;
	return 0;
}
int CZEpubReader::Close()
{
	int reCode;
	if (m_book)
	{
		if (m_curFileType == FILETYPE_TXT)
		{
			m_book->ClearVContent(true);
		}
		delete m_book;
		m_book = NULL;
	}
	if (isClear && m_epubFile )
	{
		reCode = unzClose(*m_epubFile);
		delete m_epubFile;
		m_epubFile = NULL;
	}
	else
		reCode = 0;
	if (m_rootFile != NULL)
	{
		delete[] m_rootFile;
		m_rootFile = NULL;
	}
	if (ncxFile != NULL)
	{
		delete[] ncxFile;
		ncxFile = NULL;
	}
	oebpsDir.erase(oebpsDir.begin(),oebpsDir.end());
	isClear = true;
	
	return reCode;
}

/* ����ֵ
		0  - �ɹ�
		-1 - ��ȡѹ���ļ�ʧ��
		-2 - ����xmlʧ��
*/
int CZEpubReader::OpenMETA_INF()
{

	CZLFile Metainf(m_epubFile);
	if (0 != Metainf.OpenFile("META-INF/container.xml"))
	{
		return -1;
	}
	else
	{
		//����xml
		CXMLParser par;
		if (0 != par.ParseContainer(this,&Metainf))
		{
			return -2;
		}
	}

	/*if (0 != Metainf.OpenFile("META-INF/metadata.xml"))
	{
		META_INF_metadata = false;
	}
	else
	{
		//����xml
	}

	if (0 != Metainf.OpenFile("META-INF/signatures.xml"))
	{
		META_INF_signatures = false;
	}
	else
	{
		//����xml
	}

	if (0 != Metainf.OpenFile("META-INF/encryption.xml"))
	{
		META_INF_encryption = false;
	}
	else
	{
		//����xml
	}

	if (0 != Metainf.OpenFile("META-INF/rights.xml"))
	{
		META_INF_rights = false;
	}
	else
	{
		//����xml
	}

	if (0 != Metainf.OpenFile("META-INF/manifest.xml"))
	{
		META_INF_manifest = false;
	}
	else
	{
		//����xml
	}*/
	return 0;
}
int CZEpubReader::CreateRootFile(int length)
{
	if (length <= 0)
	{
		return 2;
	}
	if (m_rootFile != NULL)
	{
		delete[] m_rootFile;
		m_rootFile = NULL;
	}
	m_rootFile = new char[length + 1];
	if (m_rootFile == NULL)
	{
		return 0;
	}
	return 1;
}
int CZEpubReader::putRootFile(const char *rootfile)
{
	int length = strlen(rootfile);

    if (!CreateRootFile(length))
    {
        return -1; 
    }
	memcpy(m_rootFile,rootfile,length);
	m_rootFile[length] = 0;

// 	char* pos = strstr(m_rootFile,"/");
// 	if (pos != NULL)
// 	{
// 		int length = pos - m_rootFile;
// 		char* temp = new char[length +1];
// 		if (temp == NULL)
// 		{
// 			return -1;
// 		}
// 		memcpy(temp,m_rootFile,length);
// 		temp[length] = 0;
// 		//string oebps(temp);
// 		oebpsDir.assign(temp);
// 		
// // 		oebps.append("/toc.ncx");
// //		int l = oebps.length();
// 		ncxFile = new char[length + strlen("/toc.ncx") + 1];
// 		if (ncxFile == NULL)
// 		{
// 			return -1;
// 		}
// 		memset(ncxFile,0,length + strlen("/toc.ncx") + 1);
// 		memcpy(ncxFile,temp,length);
// 		strcat(ncxFile,"/toc.ncx");
// 		delete[] temp;
// 		//ncxFile[length + strlen("/toc.ncx")] = 0;
// 	
// 	}
// 	else
// 	{
// 		ncxFile = new char[8];
// 		if (ncxFile == NULL)
// 		{
// 			return -1;
// 		}
// 		memset(ncxFile,0,8);
// 		memcpy(ncxFile,"toc.ncx",7);
// 	}

	return 0;
}
/*����ֵ 0 �ɹ�
         -1 ��ѹ����������ʧ��
		 -2 ����XMLʧ��
*/
int CZEpubReader::OpenRootFile()
{

	CZLFile rootFile(m_epubFile);
	if (0 != rootFile.OpenFile(m_rootFile))
	{
		return -1;
	}
	else	
	{
		//����xml
		CXMLParser par;
		par.putOebpsDir(rootFile.GetCurDir());
		if (0 != par.ParseRootFile(this,&rootFile))
		{
			return -2;
		}
	}
	return 0;
}

int CZEpubReader::OpenOebpsFile()
{

	CZLFile zNcxFile(m_epubFile);
	if (0 != zNcxFile.OpenFile(ncxFile))
	{
		return 0;
	}
	else
	{

		//test
// 		FILE* temp = fopen("c:\\1.xml","wb");
// 		if (temp)
// 		{
// 			fwrite(zNcxFile.GetFileData(),1,zNcxFile.GetDataLength(),temp);
// 			fclose(temp);
// 		}
		//����xml
		CXMLParser par;
		par.putOebpsDir(zNcxFile.GetCurDir());
		if (0 != par.ParseObepsFile(this,&zNcxFile))
		{
			return -2;
		}
	}
	return 0;
}

/*int CZlibFbreader::ReadFileAndParser( const char* file )
{
	byte* buffer = NULL;
	int length;

	
	if (0 != ReadOneFileToMem(m_epbuFile,(char*)file,&buffer,&length))
	{
		if (buffer != NULL)
		{
			free(buffer);
		}
		
		return -1;
	}
	else
	{
		m_book.m_curHtml.PutData(buffer,length);
		m_book.m_curHtml.ParserHtml();
	}
	//free(buffer);
}*/

void CZEpubReader::FreeFile(void)
{
	
}

// ͨ��order��ȡ�ļ�
int CZEpubReader::ReadFileByOrder(int order)
{
	string file;
	if (m_book->m_bookChapter->GetFileByOrder(order,file))
	{
		return ReadFileByFilePath(file);
	}
	else
		return -1;
	return 0;
}

int CZEpubReader::ReadFileByFilePath( string file )
{
    LOGI("CZEpubReader::ReadFileByFilePath begin | file:%s",file.c_str());
	int re = m_book->m_curFile->OpenFile(file.c_str());
	if (-1 == re)
	{
		return -2;
	}
		else if (OPENZIPFILE_LASTOPEN == re)
	{//�ϴδ򿪣�����Ҫ�ٴν�����
		if (!m_book->m_curFile->m_isAnchor)
		{
			//����ê��ҳ�棬����Ҫ�ظ�����
			return 1;
		}
	}
	else if (re == OPENZIPFILE_NOT_EXIST)
	{
		return OPENZIPFILE_NOT_EXIST;
	}
	m_book->m_curFile->PutOebpsDir(oebpsDir);
	return m_book->parser(file);
}

// ��ý���ļ���ͼ��
int CZEpubReader::ReadMediaFileByFilePath(string file,myBYTE* data,int& datalength)
{
	CZLFile imgfile;

	if (-1 == imgfile.OpenFile(file.c_str()))
	{
		return -1;
	}
	datalength = imgfile.GetDataLength();
	data = new myBYTE[datalength + 1];
	if (data == NULL)
	{
		return -2;
	}
	memset(data,0,datalength + 1);
	memcpy(data,imgfile.GetFileData(),datalength);

	return 0;
}

void CZEpubReader::SetEpubOpp( unzFile* epubfile )
{
	isClear = false;
	this->m_epubFile = epubfile;
	this->m_book = new CBook(m_epubFile);
}

void CZEpubReader::PutNcxFile( string& ncx )
{
	int length = ncx.size();
	this->ncxFile = new char[length + 1];
	memset(this->ncxFile,0,length + 1);
	memcpy(this->ncxFile,ncx.c_str(),length);
}

int CZEpubReader::OpenTxt(char *filepath)
{
	EncodingType encode = CheckFileEncodingType(filepath);
	FILE* txt = fopen(filepath,"rb");
	if (txt == NULL)
	{
		return -1;
	}
	MSI msi;
	m_chapterorder = 0;
	myBYTE* buff = NULL;
	int filelength;
	fseek(txt,0,SEEK_END);
	filelength=ftell(txt);
	fseek(txt,0,SEEK_SET);
	buff = new myBYTE[filelength+1];
	if (buff == NULL)
	{
		fclose(txt);
		return -1;
	}
	memset(buff,0,filelength + 1);
	size_t l = fread(buff,1,filelength,txt);
	fclose(txt);
	if (m_book == NULL)
	{
	m_book = new CBook(m_epubFile);
	if (m_book == NULL)
	{
		return -1;
	}
	}
	

	bool isUnicode = false;
		switch(encode)
		{
		case UTF_8:
			m_book->m_curFileEncodingtype = 1;
			break;
		case ANSI:
			m_book->m_curFileEncodingtype = 2;
			break;
		default:
			m_book->m_curFileEncodingtype = 0;
		isUnicode = true;
			break;
		}
	myBYTE* tempbuff = buff;
	int lastoffset = 0;
	int i;
	int tiao;
	for (i=0;i<filelength;i++)
	{
		bool isfind= false;
		if (encode == UTF16_LE)//unicode
		{
			if (i + 3 < filelength)
			{
				if (tempbuff[i] == 0x0d && tempbuff[i+1] == 0x00 &&tempbuff[i+2] == 0x0a && tempbuff[i+3] == 0x00)
				{
					isfind = true;
					tiao = 4;
				}
				else if (tempbuff[i] == 0x0d && tempbuff[i+1] == 0x00)
				{
					isfind = true;
					tiao = 2;
				}
				else if (tempbuff[i] == 0x0a && tempbuff[i+1] == 0x00)
				{
					isfind = true;
					tiao = 2;
				}
			}
		}
		else if (encode == UTF16_BE)//unicode big
		{
			if (i + 3 < filelength)
			{
				if (tempbuff[i] == 0x00 && tempbuff[i+1] == 0x0d &&tempbuff[i+2] == 0x00 && tempbuff[i+3] == 0x0a)
				{
					isfind = true;
					tiao = 4;
				}
				else if (tempbuff[i] == 0x00 && tempbuff[i+1] == 0x0d)
				{
					isfind = true;
					tiao = 2;
				}
				else if (tempbuff[i] == 0x00 && tempbuff[i+1] == 0x0a)
				{
					isfind = true;
					tiao = 2;
				}
			}
		}
		else
		{
			if (i + 1 < filelength)
			{
				if (tempbuff[i] == 0x0d && tempbuff[i+1] == 0x0a)
				{
					isfind = true;
					tiao = 2;
				}
				else if(tempbuff[i] == 0x0d)
				{
					isfind = true;
					tiao = 1;
				}
				else if (tempbuff[i] == 0x0a)
				{
					isfind = true;
					tiao = 1;
				}
			}
		}
		if (isfind)
		{
			Content* cont = new Content();
			if (cont == NULL)
			{
				return -1;
			}
			cont->isHaveStyle = false;
			//cont->isFirst = false;
			cont->titleType = 0;
			cont->contentType = CT_Text;
			cont->before = cont->after = NULL;
			cont->titleType = 0;
			cont->isP_start = true;
			cont->isP_end = true;
			int length;
				length = i - lastoffset/* + 4*/;
			i+=tiao;
			
			myBYTE* chText = new myBYTE[length+1];
			if (chText == NULL)
			{
				return -1;
			}
			cont->text = new myWcharT[length+1];
			if (cont->text == NULL)
			{
				return -1;
			}
			memset(chText,0,length+1);
			memset(cont->text,0,length*sizeof(myWcharT) + sizeof(myWcharT));
			memcpy(chText,tempbuff+lastoffset,length);
			//ת��
			CStringConver strcon;
			if (encode == UTF_8)
			{
				strcon.Utf8ToUnicodeBig((char*)chText,length,cont->text,length*sizeof(myWcharT));
			}
			else if (encode == ANSI)
			{
				strcon.GbkToUnicodeBig((char*)chText,length,cont->text,length*sizeof(myWcharT));
			}
			else if (encode == UTF16_BE)
			{
				strcon.UnicodeSmallToUnicodeBig((char*)chText,length,cont->text,length*sizeof(myWcharT));
			}
			else
			{
#ifdef _WIN32
				memcpy(cont->text,chText,length);
#else
			for (int i=0,j=0;i < length;i+=2,j++)
			{
				wchar_t outText;
				char* tempchar = (char*)&outText;
					tempchar[0] = chText[i];
					tempchar[1] = chText[i+1];
				cont->text[j] = outText;
			}
#endif // _WIN32
			}
			cont->txt_SrcPtext = cont->text;
			if (chText)
			{
				delete[] chText;
				chText = NULL;
			}
			
			//����ǰ��Ŀո�
			int befor;
			myWcharT kongge = 0x0020;
			myWcharT kongge2 =0x3000;
			myWcharT huanhang = L'\n';
			myWcharT* temptext = cont->text;
			int textLegth = myWcslen(cont->text);
			for (befor = 0;befor<textLegth ;befor++)
			{
				if (temptext[befor] != kongge && temptext[befor] != kongge2 && temptext[befor] != huanhang)
				{
					break;
				}
			}
			if (befor > 0)
			{
				cont->before = new myWcharT[befor + 1];
				if (cont->before != NULL)
				{
					memset(cont->before,0,sizeof(myWcharT)*(befor+1));
					memcpy(cont->before,temptext,befor*sizeof(myWcharT));
				}
				cont->text = cont->txt_SrcPtext + befor;		
			}
			m_book->m_vContent.push_back(cont);
			//�ж�Ŀ¼
			AutoTakeChapterinfo(encode,cont,filepath,m_book->m_vContent.size()-1,msi);
			lastoffset = i;
			i--;
		}
	}
	if (lastoffset < filelength)
	{
		Content* cont = new Content();
		if (cont == NULL)
		{
			return -1;
		}
		int length = filelength-lastoffset;
		myBYTE* chText = new myBYTE[length+1];
		if (chText == NULL)
		{
			return -1;
		}
		cont->text = new myWcharT[length+1];
		if (cont->text == NULL)
		{
			return -1;
		}
		memset(chText,0,length+1);
		memset(cont->text,0,length*sizeof(myWcharT) + sizeof(myWcharT));
		memcpy(chText,tempbuff+lastoffset,length);
		
		//ת��
		CStringConver strcon;
		if (m_book->m_curFileEncodingtype == 1)
		{
			strcon.Utf8ToUnicodeBig((char*)chText,length,cont->text,length*sizeof(myWcharT));
		}
		else if (m_book->m_curFileEncodingtype == 2)
		{
			strcon.GbkToUnicodeBig((char*)chText,length,cont->text,length*sizeof(myWcharT));
		}
		else if (encode == UTF16_BE)
		{
			strcon.UnicodeSmallToUnicodeBig((char*)chText,length,cont->text,length*sizeof(myWcharT));
		}
		else
		{
#ifdef _WIN32
			memcpy(cont->text,chText,length);
#else
			for (int i=0,j=0;i < length;i+=2,j++)
			{
				wchar_t outText;
				char* tempchar = (char*)&outText;
				tempchar[0] = chText[i];
				tempchar[1] = chText[i+1];
				cont->text[j] = outText;
			}
#endif // _WIN32
		}
		cont->txt_SrcPtext = cont->text;
		if (chText)
		{
			delete[] chText;
			chText = NULL;
		}
		//����ǰ��Ŀո�
		int befor;
		myWcharT kongge = 0x0020;
		myWcharT kongge2 =0x3000;
		myWcharT huanhang = L'\n';
		myWcharT* temptext = cont->text;
		int textLegth = myWcslen(cont->text);
		for (befor = 0;befor<textLegth ;befor++)
		{
			if (temptext[befor] != kongge && temptext[befor] != kongge2 && temptext[befor] != huanhang)
			{
				break;
			}
		}
		if (befor > 0)
		{
			cont->before = new myWcharT[befor + 1];
			if (cont->before != NULL)
			{
				memset(cont->before,0,sizeof(myWcharT)*(befor+1));
				memcpy(cont->before,temptext,befor*sizeof(myWcharT));
			}
			cont->text = cont->txt_SrcPtext + befor;		
		}
		m_book->m_vContent.push_back(cont);
	}
	delete[] buff;
	buff = NULL;
	string name(filepath);
	m_book->m_bookSpine->PushBackSpine(name);
	m_book->m_FIleAnchorContentNumber.insert(pair<string,MSI>(name,msi));
	return 0;
}

int CZEpubReader::OpenFileByFilePath(string file)
{
	return m_book->OpenFileByFilePath(file);
}

int CZEpubReader::AutoTakeChapterinfo(EncodingType encode,Content* cont,char* filepath, int contentID,MSI& msi)
{
	string strpat,re;
	strpat.assign(".*��.*��.*");
	CStringConver sc;
	int Textlen = myWcslen(cont->text);
	char* szText = new char[Textlen*2+1];
	if (szText == NULL)
	{
		return 0;
	}
	memset(szText,0,Textlen*2+1);
	sc.UnicodeBigToGbk(cont->text,Textlen*sizeof(wchar_t),szText,Textlen*2);
	
	regex_t results;
	int nErrCode = 0;
	if(regcomp(&results,strpat.c_str(),0) == 0)
	{
		if ((nErrCode = regexec(&results,szText,0,NULL,0)) == 0)
		{

			//LOGI("AutoTakeChapterinfo |regexec ");
			re.assign(szText);
			if (Textlen < 40)
			{
				m_book->m_bookChapter->PutInfo(m_chapterorder++,cont->text,filepath,1);
				msi.insert(pair<string,int>(re,contentID));
			}
		}
	}

	regfree(&results);
	delete[] szText;
	/*
	match_results results;
	rpattern pat(strpat.c_str());
	match_results::backref_type br = pat.match(content, results );
	if( br.matched ) {
		re = results.backref(0).str();
		//ȥ����λ�ո񣬻��з�
		m_Trim(re);
		if (re.length() < 40)
		{
			
			m_book->m_bookChapter->PutInfo(m_chapterorder++,re,filepath,1);

			msi.insert(pair<string,int>(re,contentID));
		}

	} */
	return 0;
}
