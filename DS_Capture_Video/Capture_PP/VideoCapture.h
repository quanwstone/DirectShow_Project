#pragma once

#include<vector>
#include"SampleCallBack.h"

class CVideoCapture
{
public:
	CVideoCapture();
	~CVideoCapture();
	
private:
	ICreateDevEnum	*m_pDevEnum;
	IEnumMoniker	*m_pEnumMoniker;

	IBaseFilter		*m_pCaptureFilter;
	IBaseFilter		*m_pNullRenderFilter;
	IBaseFilter		*m_pGrabberFilter;
	IBaseFilter		*m_pSmartTeeFilter;

	ISampleGrabber	*m_pSampleGrabber;
	IGraphBuilder	*m_pGraphBuilder;
	ICaptureGraphBuilder2	*m_pCaptureGraph;
	IMediaControl	*m_pMediaControl;
	IVideoWindow	*m_pVideoWindow;

	IAMStreamConfig *m_pConfig;
	CSampleCallBack *m_pSampleCallBack;

	std::vector<char *>m_vecDevList;

	int	m_iHeigh;
	int m_iWidth;
	int m_iBitRate;
	int m_iFPS;
private:
	char *WCharConvChar(WCHAR *in_wchStr);
	
	IBaseFilter	*GetSpecifiedCaptureFilter(char *in_chFilterName);
	
	IPin *FindPin(IBaseFilter *in_pFilter, PIN_DIRECTION in_dir);

	BOOL ConnectPins();

	HRESULT SetWindowDisplay();

	HRESULT GetVideoParams();

	HWND m_hwnd;

	void _FreeMediaType(AM_MEDIA_TYPE& mt);

	void _DeleteMediaType(AM_MEDIA_TYPE *pmt);
public:

	/*
	说明：初始化视频设备
	参数：in_hwnd用于窗口显示的句柄
	返回值：成功返回TRUE，失败返回FALSE.
	*/	
	BOOL InitVideoDevice(HWND in_hwnd);

	/*
	说明：获取视频设备列表
	参数：out_chDevList设备列表信息
	返回值：成功返回TRUE，失败返回FALSE.
	*/
	BOOL GetDeviceList(char *out_chDevList[]);

	/*
	说明：打开设备
	参数：in_chDevName设备名称
	返回值：成功返回TRUE，失败返回FALSE.
	*/
	BOOL OpenDevice(char *in_chDevName);

	/*
	说明：开始采集
	返回值：成功返回TRUE，失败返回FALSE.
	*/
	BOOL Start();

	/*
	说明：关闭设备
	返回值：成功返回TRUE，失败返回FALSE.
	*/
	BOOL CloseDevice();
};

