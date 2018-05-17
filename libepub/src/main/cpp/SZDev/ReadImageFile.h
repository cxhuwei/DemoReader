#ifndef _READIMAGEFILE_H
#define _READIMAGEFILE_H
#include "ZLFile.h"
#include "../SZDev/SZImage.h"
// #include "png/png.h"
typedef struct _ImgInfo
{
	int width;
	int height;
	int bpp;
} ImgInfo,*pImgInfo;

enum e_imageFileType{BMP_IMAGE=1,JPG_IMAGE,PNG_IMAGE,GIF_IMAGE,OTHER};
class CReadImageFile
{
public:
	CReadImageFile(void);
	~CReadImageFile(void);
	int ReadImageInfo(CZLFile* file,pImgInfo p_imginfo);
	int ReadImage(CZLFile* file,SZImageBase*& image,short flag=0);//flag:0-正式读图片数据；1-只读图片相关信息
private:
#ifndef USE_CXIMAGE
	int ReadJpeg(CZLFile* file,SZImageBase*& image,short flag=0);
	int ReadPng( CZLFile* file,SZImageBase*& image,short flag=0);
	int ReadGif(CZLFile* file, SZImageBase*& image,short flag=0);
#endif
	
	e_imageFileType GetImageType(myBYTE* buff);
	//infor
	int m_width;
	int m_height;
	int m_bpp;
};

#endif
