#include "StdAfx.h"
#include "MAlgorithm.h"
#include "ReadImageFile.h"

// #include "jpeg/jpeglib.h"
// #include "png/PngImage.h"
// #include "gif/gif_lib.h"
#include <setjmp.h>
#include "jpeglib.h"

#ifdef USE_CXIMAGE
	#include "ximage.h"
#else
	#include "PngImage.h"
	#include "gif_lib.h"
#endif

CReadImageFile::CReadImageFile(void)
{
}

CReadImageFile::~CReadImageFile(void)
{
}

int CReadImageFile::ReadImageInfo( CZLFile* file,pImgInfo p_imginfo )
{
	SZImageBase* temp = NULL;
	if (ReadImage(file,temp,1) != 0)
	{
		return -1;
	}
	else
	{
		p_imginfo->bpp = m_bpp;
		p_imginfo->width = m_width;
		p_imginfo->height = m_height;
		return 0;
	}
}
e_imageFileType CReadImageFile::GetImageType(myBYTE* buff)
{

	e_imageFileType flag = OTHER;
	if(buff[0] == 0x42 && buff[1] == 0x4D)
	{
		//bmp
		flag = BMP_IMAGE;
	}
	else if( buff[0]== 0xFF && (buff[1] == 0xD8 || buff[1] == 0xC4 || buff[1] == 0xC0 || buff[1] == 0xDA || buff[1] == 0xFE || buff[1] == 0xD9) )
	{
		//jpeg
		flag = JPG_IMAGE;
	}
	else if( buff[0] == 0x89
		&& buff[1] == 0x50
		&& buff[2] == 0x4E
		&& buff[3] == 0x47
		&& buff[4] == 0x0D
		&& buff[5] == 0x0A
		&& buff[6] == 0x1A
		&& buff[7] == 0x0A )
	{
		//png
		flag = PNG_IMAGE;
	}
	else if( buff[0] == 0x47
		&& buff[1] == 0x49
		&& buff[2] == 0x46
		/*&& buff[3] == 0x38
		&& buff[4] == 0x39
		&& buff[5] == 0x61*/ )
	{
		//gif
		flag = GIF_IMAGE;
	}
	else
		flag = OTHER;
	return flag;
}
int CReadImageFile::ReadImage( CZLFile* file,SZImageBase*& image,short flag)
{
	if (file->GetFileData() == NULL)
	{
		return -1;
	}
// 	char* ext = file->GetExtension();
// 	char* lext = m_ToLower(ext);

	e_imageFileType imageType = GetImageType(file->GetFileData());

#ifdef USE_CXIMAGE
    CxImage ima111;    
	ENUM_CXIMAGE_FORMATS cxFormat = CXIMAGE_FORMAT_UNKNOWN;
//     if (strcmp(lext,"png") == 0)
// 	{
//         ima111.Decode(file->GetFileData(), file->GetDataLength(),CXIMAGE_FORMAT_PNG);         
// 		//return ReadPng(file,image,flag);
// 	}
// 	else if (strcmp(lext,"jpeg") == 0 || strcmp(lext,"jpg") == 0)
// 	{
//         
//         ima111.Decode(file->GetFileData(), file->GetDataLength(),CXIMAGE_FORMAT_JPG);
// 		//return ReadJpeg(file,image,flag);
// 	}
// 	else if (strcmp(lext,"gif") == 0)
// 	{
//         ima111.Decode(file->GetFileData(), file->GetDataLength(),CXIMAGE_FORMAT_GIF);
// 		//return ReadGif(file,image,flag);
// 	}
	switch (imageType)
	{
	case BMP_IMAGE:
		cxFormat = CXIMAGE_FORMAT_BMP;
		break;
	case JPG_IMAGE:
		cxFormat = CXIMAGE_FORMAT_JPG;;
		break;
	case PNG_IMAGE:
		cxFormat = CXIMAGE_FORMAT_PNG;
		break;
	case GIF_IMAGE:
		cxFormat = CXIMAGE_FORMAT_GIF;
		break;
	}
	ima111.Decode(file->GetFileData(), file->GetDataLength(),cxFormat);

	if (!ima111.IsValid())
	{
		return -1;
	}
    
    m_width     = ima111.GetWidth();
    m_height    = ima111.GetHeight();
    m_bpp       = ima111.GetBpp();	 
	
    if (flag == 1)	
		return 0;	

	switch(m_bpp)
	{
	case 1:
		image = new SZImageBit1();
		break;
	case 4:
		image = new SZImageBit4();
		break;
	case 8:
		image = new SZImageBit8();
		break;
	case 24:
		image = new SZImageBit24();
		break;
	case 32:
		image = new SZImageBit32();
		break;
	}
	if (!image)
	{
		return -1;
	}
    
	image->Init(m_width,m_height);
	
	if (m_bpp <24)
	{

		//设置调色板
		for (int i = 0; i < pow((float)2,(float)m_bpp);i++)
		{
			BYTE r;
			BYTE g;
			BYTE b;
			ima111.GetPaletteColor(i,&r,&g,&b);
			myRGBQUAD paletteColor;
			paletteColor.rgbRed = r;
			paletteColor.rgbGreen = g;
			paletteColor.rgbBlue = b;
			image->SetGrayPalette(i,paletteColor);
		}

	}

	image->SetBits(ima111.GetBits(),0,ima111.GetSize());

	return 0;
#else
	if (strcmp(lext,"png") == 0)
	{
		return ReadPng(file,image,flag);
	}
	else if (strcmp(lext,"jpeg") == 0 || strcmp(lext,"jpg") == 0)
	{
		return ReadJpeg(file,image,flag);
	}
	else if (strcmp(lext,"gif") == 0)
	{
		return ReadGif(file,image,flag);
	}
	else
		return -1;
#endif	
}

#ifndef USE_CXIMAGE
/* decompression jpeg  */
struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */

	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}
int CReadImageFile::ReadJpeg( CZLFile* file,SZImageBase*& image,short flag)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	int nAdjust; // 用于字节对齐
	myBYTE* data;
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
	/* If we get here, the JPEG code has signaled an error.
	* We need to clean up the JPEG object, close the input file, and return.
		*/
		jpeg_destroy_decompress(&cinfo);
		return -1;
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);
	//jpeg_stdio_src(&cinfo,file->GetFileData(),file->GetDataLength());
	jpeg_mem_src(&cinfo,file->GetFileData(),file->GetDataLength());
	(void) jpeg_read_header(&cinfo, true);
	int bpp = cinfo.num_components * cinfo.data_precision;
	if (flag == 1)
	{//只读信息
		m_width= cinfo.image_width;
		m_height = cinfo.image_height;
		m_bpp = bpp;
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}
	nAdjust = cinfo.image_width*cinfo.num_components%4;
	if (nAdjust) nAdjust = 4-nAdjust;
	data = new myBYTE[(cinfo.image_width*cinfo.num_components+nAdjust)*cinfo.image_height];	
	if (data == NULL)
	{
		return -1;
	}
//	cinfo.jpeg_color_space = JCS_RGB;
	(void) jpeg_start_decompress(&cinfo);

	
	switch (bpp)
	{
	case 8:
		image = new SZImageBit8();			
		break;
	case 24:
		image = new SZImageBit24();
		break;
	case 32:
		image = new SZImageBit32();
	default:
		break;
	}
	JSAMPROW row_pointer[1];
	while(cinfo.output_scanline < cinfo.output_height)
	{
		row_pointer[0] = &data[(cinfo.output_height - cinfo.output_scanline-1)*(cinfo.image_width*cinfo.num_components+nAdjust)];
		jpeg_read_scanlines(&cinfo,row_pointer,1);
	}
	if (image)
	{
		image->Init(cinfo.image_width,cinfo.image_height);
		if (cinfo.num_components==3)
		{
			// 调整rgb顺序
			for (int j=0;j<cinfo.image_height;j++)
				for (int i = 0;i<cinfo.image_width;i++)
				{
					myBYTE red = data[j*(cinfo.image_width*cinfo.num_components+nAdjust)+i*3];
					data[j*(cinfo.image_width*cinfo.num_components+nAdjust)+i*3] = data[j*(cinfo.image_width*cinfo.num_components+nAdjust)+i*3+2];
					data[j*(cinfo.image_width*cinfo.num_components+nAdjust)+i*3+2] = red;
				}
		}
		if (cinfo.num_components == 4)
		{
			for (int j=0;j<cinfo.image_height;j++)
				for (int i = 0;i<cinfo.image_width;i++)
				{
					myBYTE red = data[j*(cinfo.image_width*cinfo.num_components+nAdjust)+i*4];
					data[j*(cinfo.image_width*cinfo.num_components+nAdjust)+i*4] = data[j*(cinfo.image_width*cinfo.num_components+nAdjust)+i*4+2];
					data[j*(cinfo.image_width*cinfo.num_components+nAdjust)+i*4+2] = red;
				}
		}
		image->SetBits(data,0,(cinfo.image_width*cinfo.num_components+nAdjust)*cinfo.image_height);
		//image->writeBMPFile("d:\\bmp2.bmp");
	}
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	delete[] data;
	return 0;
}

int CReadImageFile::ReadPng( CZLFile* file,SZImageBase*& image,short flag)
{
	PngImage png;
	if (!png.load(file->GetFileData(),file->GetDataLength()))
	{
		return -1;
	}
	
	if (flag == 1)
	{
		m_width  = png.getWidth();
		m_height = png.getHeight();
		m_bpp    = 24;
		return 0;
	}
	
	image = new SZImageBit24();
	if (!image)
	{
		return -1;
	}
	
	myRGBQUAD bgColor={0,0,0,0};
	m_width  = png.getWidth();
	m_height = png.getHeight();
	image->Init(png.getWidth(),png.getHeight());
	
	unsigned char* data = png.getBGRA();
	SZImageBit24 *ptempImag=(SZImageBit24 *)image;	
	for (long y=0;y<m_height;y++)
	{
		for (long x=0;x<m_width;x++)
		{ 
			memcpy(&bgColor,(data+((m_height-1-y)*m_width+x)*4),4);	
			ptempImag->SetDesPixel(x,y,bgColor,255); 
		}
	}
	return 0;
}
int gif_input_cb(GifFileType* gif, GifByteType* bytes, int size) 
{
	static long offer(0);
	CZLFile* file = (CZLFile*)gif->UserData;
	memcpy(bytes,file->GetFileData() + offer,size);
	offer += size;
	return size;
}
int CReadImageFile::ReadGif( CZLFile* file, SZImageBase*& image ,short flag)
{
	static int
	BackGround = 0,
	InterlacedOffset[] = { 0, 4, 2, 1 }, /* The way Interlaced image should. */
	InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */
	GifFileType* GifFile;
	int	i, j, Size, Row, Col, Width, Height, ExtCode;
	GifRecordType RecordType;
	GifByteType *Extension;
	GifRowType *ScreenBuffer;
	GifFile = DGifOpen(file, gif_input_cb);
	if (GifFile == NULL) 
	{
		     PrintGifError();
			 return -1;
	}
	if (flag == 1)
	{
		m_width = GifFile->SWidth;
		m_height = GifFile->SHeight;
		m_bpp = 24;
		DGifCloseFile(GifFile);
		return 0;

	}
	if ((ScreenBuffer = (GifRowType *)malloc(GifFile->SHeight * sizeof(GifRowType *))) == NULL)
		return -1;
		

	Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
	if ((ScreenBuffer[0] = (GifRowType) malloc(Size)) == NULL) /* First row. */
		return -1;

	for (i = 0; i < GifFile->SWidth; i++)  /* Set its color to BackGround. */
		ScreenBuffer[0][i] = GifFile->SBackGroundColor;
	for (i = 1; i < GifFile->SHeight; i++) {
		/* Allocate the other rows, and set their color to background too: */
		if ((ScreenBuffer[i] = (GifRowType) malloc(Size)) == NULL)
			return -1;

		memcpy(ScreenBuffer[i], ScreenBuffer[0], Size);
	}
	/* Scan the content of the GIF file and load the image(s) in: */
	do {
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
			PrintGifError();
			exit(EXIT_FAILURE);
		}
		switch (RecordType) {
		case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(GifFile) == GIF_ERROR) {
				PrintGifError();
				exit(EXIT_FAILURE);
			}
			Row = GifFile->Image.Top; /* Image Position relative to Screen. */
			Col = GifFile->Image.Left;
			Width = GifFile->Image.Width;
			Height = GifFile->Image.Height;
			if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
				GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) {
					exit(EXIT_FAILURE);
			}
			if (GifFile->Image.Interlace) {
				/* Need to perform 4 passes on the images: */
				for (i = 0; i < 4; i++)
					for (j = Row + InterlacedOffset[i]; j < Row + Height;
						j += InterlacedJumps[i]) {
							if (DGifGetLine(GifFile, &ScreenBuffer[j][Col],
								Width) == GIF_ERROR) {
									PrintGifError();
									exit(EXIT_FAILURE);
							}
					}
			}
			else {
				for (i = 0; i < Height; i++) {
					if (DGifGetLine(GifFile, &ScreenBuffer[Row++][Col],
						Width) == GIF_ERROR) {
							PrintGifError();
							exit(EXIT_FAILURE);
					}
				}
			}
			break;
		case EXTENSION_RECORD_TYPE:
			/* Skip any extension blocks in file: */
			if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) {
				PrintGifError();
				exit(EXIT_FAILURE);
			}
			while (Extension != NULL) {
				if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) {
					PrintGifError();
					exit(EXIT_FAILURE);
				}
			}
			break;
		case TERMINATE_RECORD_TYPE:
			break;
		default:		    /* Should be traps by DGifGetRecordType. */
			break;
		}
	} while (RecordType != TERMINATE_RECORD_TYPE);

	/* Lets dump it - set the global variables required and do it: */
	BackGround = GifFile->SBackGroundColor;
	ColorMapObject* ColorMap = (GifFile->Image.ColorMap
		? GifFile->Image.ColorMap
		: GifFile->SColorMap);
	if (ColorMap == NULL) {
		exit(EXIT_FAILURE);
	}
	
	int nAdjust = GifFile->SWidth*3%4;
	if (nAdjust) nAdjust = 4-nAdjust;
	unsigned char *Buffer, *BufferP;
	myBYTE* data = new myBYTE[(GifFile->SWidth * 3 + nAdjust) * GifFile->SHeight ];
	int off(0);
	memset(data,0,(GifFile->SWidth * 3 + nAdjust) * GifFile->SHeight);
	GifRowType GifRow;
	static GifColorType *ColorMapEntry;
	if ((Buffer = (unsigned char *) malloc(GifFile->SWidth * 3)) == NULL)
		return -1;
	for (i = 0; i < GifFile->SHeight; i++) {
		off = (GifFile->SWidth * 3 + nAdjust) * GifFile->SHeight - (i+1)*(GifFile->SWidth * 3 + nAdjust);
		GifRow = ScreenBuffer[i];
		for (j = 0, BufferP = Buffer; j < GifFile->SWidth; j++) {
			ColorMapEntry = &ColorMap->Colors[GifRow[j]];
			*BufferP++ = ColorMapEntry->Blue;
			*BufferP++ = ColorMapEntry->Green;
			*BufferP++ = ColorMapEntry->Red;
		}
		memcpy(data + off,Buffer,GifFile->SWidth*3);
	}
	image = new SZImageBit24();
	if (image)
	{
		image->Init(GifFile->SWidth,GifFile->SHeight);
		image->SetBits(data,0,GifFile->SWidth*GifFile->SHeight*3);
	}
	else
		return -1;
	
	if (DGifCloseFile(GifFile) == GIF_ERROR) {
		PrintGifError();
		exit(EXIT_FAILURE);
	}
	return 0;
	  
}

#endif
