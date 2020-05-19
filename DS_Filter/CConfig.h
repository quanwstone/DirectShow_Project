#pragma once
#include<dshow.h>

#define FOURCC_YUY2  0x32595559

#define BITCOUNT     16

#define VCAM_VIDEO_HEADER(DX,DY, INDEX) \
    static VIDEOINFOHEADER VCAM_VideoHeader_##INDEX = \
	{\
        0,0,0,0,                            /* RECT  rcSource; */\
        0,0,0,0,                            /* RECT  rcTarget; */\
        DX*DY*30*BITCOUNT/8,               /* DWORD dwBitRate;*/\
        0L,                                 /* DWORD dwBitErrorRate;   */\
        333667,                             /* REFERENCE_TIME  AvgTimePerFrame (30 FPS); */\
        sizeof (BITMAPINFOHEADER),       /* DWORD biSize;*/\
        DX,                                 /* LONG  biWidth;*/\
        DY,                                /* LONG  biHeight; -biHeight indicate TopDown for RGB*/\
        1,                         /* WORD  biPlanes;*/\
        BITCOUNT,                 /* WORD  biBitCount;*/\
        FOURCC_YUY2,                 /* DWORD biCompression;*/\
        DX*DY*BITCOUNT/8,         /* DWORD biSizeImage;*/\
        0,                         /* LONG  biXPelsPerMeter;*/\
        0,                         /* LONG  biYPelsPerMeter;*/\
        0,                         /* DWORD biClrUsed;*/\
        0                          /* DWORD biClrImportant;*/\
    }

/////
#define VCAM_MEDIA_TYPE(DX,DY, INDEX) \
	VCAM_VIDEO_HEADER(DX,DY, INDEX); \
	static AM_MEDIA_TYPE VCAM_MediaType_##INDEX = \
	{\
		MEDIATYPE_Video,  /*majortype*/\
		MEDIASUBTYPE_YUY2, /*subtype*/\
		TRUE, /*bFixedSizeSamples*/\
		FALSE, /*bTemporalCompression*/\
		DX*DY*BITCOUNT/8, /*lSampleSize */\
		FORMAT_VideoInfo, /*formattype*/\
		NULL, /*pUnk*/\
		sizeof(VIDEOINFOHEADER), /*cbFormat*/\
		(BYTE*)&VCAM_VideoHeader_##INDEX /*pbFormat*/\
    }

///////
VCAM_MEDIA_TYPE(1280, 720, 1);
//VCAM_MEDIA_TYPE(1920, 1080, 2);
//VCAM_MEDIA_TYPE(720, 405, 3);
//VCAM_MEDIA_TYPE(640, 480, 4);
//VCAM_MEDIA_TYPE(320, 240, 5);

static AM_MEDIA_TYPE* VCamAllMediaTypes[] =
{
	&VCAM_MediaType_1,
	//&VCAM_MediaType_2,
	//&VCAM_MediaType_3,
	//&VCAM_MediaType_4,
	//	&VCAM_MediaType_5,
};