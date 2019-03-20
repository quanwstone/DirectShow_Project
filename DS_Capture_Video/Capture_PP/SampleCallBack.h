#pragma once
#include<dshow.h>
#pragma comment(lib,"Strmiids.lib")

//#include <streams.h>
//#pragma comment(lib,"winmm.lib")//getTime
/*
无法解析的外部符号 __imp__timeGetTime@0，该符号在函数 "int __stdcall DbgCheckModuleLevel(unsigned long,unsigned long)"
*/
#include"qedit.h"
//#include"mtype.h"//DelteMediaType()
#include"Utill.h"
#include"QueueTemp.h"
#include"Effect.h"
class CSampleCallBack:public ISampleGrabberCB
{
public:
	CSampleCallBack();
	~CSampleCallBack();
public:

	STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample);
	STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);

	ULONG STDMETHODCALLTYPE AddRef() { return 1; };
	ULONG STDMETHODCALLTYPE Release() { return 2; };
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);

private:
	int m_iHeight;
	int m_iWidth;
	int m_iBitRate;
	int m_iDefference;

	BYTE *m_byteBuf;

	void  GrayPicture(BYTE *in_Src);

	int ComparePicture(BYTE *in_Src, BYTE *in_Dest,int in_iLen);

public:
	
	CQueue_<BYTE *>m_queueData;

	CEffect m_oEffect;

	BOOL SetPictureInfo(int in_iH, int in_iW,int in_iB);

	BOOL SaveBmp(unsigned char *in_chSrc,int in_iLen);
};

