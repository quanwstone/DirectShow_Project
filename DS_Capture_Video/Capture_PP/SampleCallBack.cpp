#include "stdafx.h"
#include "SampleCallBack.h"


CSampleCallBack::CSampleCallBack()
{
	m_iBitRate = 0;
	m_iHeight = 0;
	m_iWidth = 0;
	m_iDefference = 0;
	m_byteBuf = nullptr;
	
}


CSampleCallBack::~CSampleCallBack()
{
	if (m_byteBuf != nullptr)
	{
		DEF_SAFE_DELETE_BUF(m_byteBuf);
	}
}

HRESULT CSampleCallBack::SampleCB(double SampleTime, IMediaSample *pSample)
{
	TRACE("In SampleCB Len=%d\n",pSample->GetSize());

	return S_OK;
}
HRESULT CSampleCallBack::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{
	if (false)
	{
		m_oEffect.Water((unsigned char*)(pBuffer), 0x0000ff, 100, m_iWidth, m_iBitRate);

		SaveBmp(pBuffer, BufferLen);
	}

	memcpy(m_byteBuf, pBuffer, BufferLen);

	return S_OK;
}
void CSampleCallBack::GrayPicture(BYTE *in_Src)
{
	int iB = m_iBitRate / 8;
	int itemp = 0;
	int iStride = m_iWidth *iB;

	for (int i = 0; i < m_iHeight; i++)
	{
		for (int j = 0; j < m_iWidth; j++)
		{
			itemp = (in_Src[i*iStride + j*iB] + in_Src[i*iStride + j*iB + 1] + in_Src[i*iStride + j*iB + 2]) / 3;

			in_Src[i*iStride + j*iB] = itemp & 0xff;
			in_Src[i*iStride + j*iB + 1] = itemp & 0xff;
			in_Src[i*iStride + j*iB + 2] = itemp & 0xff;

		}
	}
}
int CSampleCallBack::ComparePicture(BYTE *in_Src, BYTE *in_Dest,int in_iLen)
{
	int iDefferen = 0;

	//获取图像灰度值

	GrayPicture(in_Src);
	GrayPicture(in_Dest);

	for (int i = 0; i < in_iLen; i++)
	{
		int j = abs(in_Src[i] - in_Dest[i]);
		if ( j > 10)
		{
			iDefferen++;
		}
	}

	return iDefferen;
}
HRESULT CSampleCallBack::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == IID_ISampleGrabberCB)
	{
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}
	else
	{
		*ppvObject = static_cast<IUnknown*>(this);
		return S_OK;
	}
	return E_NOTIMPL;
}
BOOL CSampleCallBack::SetPictureInfo(int in_iH, int in_iW, int in_iB)
{
	m_iBitRate = in_iB;
	m_iHeight = in_iH;
	m_iWidth = in_iW;

	if (m_byteBuf != nullptr)
	{
		DEF_SAFE_DELETE_BUF(m_byteBuf);
	}
	m_byteBuf = new BYTE[in_iH * in_iW *in_iB / 8];
	memset(m_byteBuf, 0, in_iH * in_iW *in_iB / 8);

	return TRUE;
}
BOOL CSampleCallBack::SaveBmp(unsigned char *in_chSrc, int in_iLen)
{
	if (NULL == in_chSrc)
	{
		return FALSE;
	}

	FILE* lpFilePtr = NULL;
	//char lszBmpFileName[512+128];
	BITMAPFILEHEADER lBmpFileHeader;
	BITMAPINFOHEADER lBmpInfoHeader;	//绘制视频图像用到的位图信息
	int liBmpFileSize = 0;
	int nSizeImage = in_iLen;

	//得到BMP文件大小
	liBmpFileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + nSizeImage;

	//填充BMP文件头
	lBmpFileHeader.bfType = 19778;
	lBmpFileHeader.bfSize = liBmpFileSize;
	lBmpFileHeader.bfReserved1 = 0;
	lBmpFileHeader.bfReserved2 = 0;
	lBmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//填充BMP信息头
	lBmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	lBmpInfoHeader.biPlanes = 1;
	lBmpInfoHeader.biCompression = BI_RGB;
	lBmpInfoHeader.biXPelsPerMeter = lBmpInfoHeader.biYPelsPerMeter = 0;
	lBmpInfoHeader.biClrImportant = lBmpInfoHeader.biClrUsed = 0;
	lBmpInfoHeader.biSizeImage = nSizeImage;
	lBmpInfoHeader.biWidth = m_iWidth;
	lBmpInfoHeader.biHeight = m_iHeight;
	lBmpInfoHeader.biBitCount = m_iBitRate;

	char ch_Name[256] = {0};
	sprintf_s(ch_Name, "%s%d%s", "c:\\quanwei\\BMP\\PP\\", m_iDefference, ".bmp");
	//创建、写入和关闭BMP文件
	fopen_s(&lpFilePtr, ch_Name, "wb+");
	if (lpFilePtr)
	{

		fwrite(&lBmpFileHeader, sizeof(char), sizeof(BITMAPFILEHEADER), lpFilePtr);
		fwrite(&lBmpInfoHeader, sizeof(char), sizeof(BITMAPINFOHEADER), lpFilePtr);
		fwrite(in_chSrc, sizeof(char), nSizeImage, lpFilePtr);
		fclose(lpFilePtr);
	}
	
	m_iDefference++;

	return TRUE;
}