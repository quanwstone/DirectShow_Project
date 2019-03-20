#include "stdafx.h"
#include "VideoCapture.h"

CVideoCapture::CVideoCapture()
{
	m_pDevEnum = nullptr;
	m_pEnumMoniker = nullptr;
	m_pCaptureFilter = nullptr;
	m_pNullRenderFilter = nullptr;
	m_pGrabberFilter = nullptr;
	m_pSmartTeeFilter = nullptr;
	m_pCaptureGraph = nullptr;
	m_pGraphBuilder = nullptr;
	m_pMediaControl = nullptr;
	m_pVideoWindow = nullptr;
	m_pSampleCallBack = nullptr;
	m_pConfig = nullptr;

	m_iWidth = 0;
	m_iHeigh = 0;
	m_iBitRate = 0;

}


CVideoCapture::~CVideoCapture()
{
}
//初始化设备
BOOL CVideoCapture::InitVideoDevice(HWND in_hwnd)
{

	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&m_pDevEnum);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice CLSID_SystemDeviceEnum Faild.");
		return FALSE;
	}
	
	hr = m_pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &m_pEnumMoniker, 0);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice CLSID_VideoInputDeviceCategory Faild.");
		return FALSE;
	}
	
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IFilterGraph, (void **)&m_pGraphBuilder);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice CLSID_FilterGraph Faild.");
		return FALSE;
	}

	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&m_pCaptureGraph);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice CLSID_CaptureGraphBuilder2 Faild.");
		return FALSE;
	}

	hr = m_pCaptureGraph->SetFiltergraph(m_pGraphBuilder);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice SetFiltergraph Faild.");
		return FALSE;
	}

	hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice m_pMediaControl Faild.");
		return FALSE;
	}

	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&m_pNullRenderFilter);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice CLSID_NullRenderer Faild.");
		return FALSE;
	}

	hr = m_pGraphBuilder->AddFilter(m_pNullRenderFilter, L"NullRenderer");
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice AddFilter CLSID_NullRenderer Faild.");
		return FALSE;
	}
	
	hr = m_pGraphBuilder->QueryInterface(IID_IVideoWindow, (void **)&m_pVideoWindow);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice IID_IVideoWindow Faild.");
		return FALSE;
	}

	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void **)&m_pGrabberFilter);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice CLSID_SampleGrabber Faild.");
		return FALSE;
	}

	hr = m_pGraphBuilder->AddFilter(m_pGrabberFilter, L"Grabber Filter");
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice AddFilter m_pGrabberFilter Faild.");
		return FALSE;
	}

	hr = m_pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void **)&m_pSampleGrabber);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice m_pSampleGrabber Faild.");
		return FALSE;
	}
	
	AM_MEDIA_TYPE mt;
	
	ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));

	mt.majortype = MEDIATYPE_Video;
	mt.subtype = MEDIASUBTYPE_RGB32;
	mt.formattype = FORMAT_VideoInfo;
	
	hr = m_pSampleGrabber->SetMediaType(&mt);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::InitVideoDevice m_pSampleGrabber->SetMediaType Faild.");
		return FALSE;
	}

	m_pSampleCallBack = new CSampleCallBack();

	m_pSampleGrabber->SetBufferSamples(FALSE);//TRUE表示会在SampleGrabber内部创建一个buffer，通过GetCurrentBuffer获取当前帧.
	
	m_pSampleGrabber->SetOneShot(FALSE);//TRUE表示只进入CB一次 
	
	m_pSampleGrabber->SetCallback(m_pSampleCallBack, 1);//1:BufferCB,0:SampleCB.
	
	m_hwnd = in_hwnd;

	TRACE("CVideoCapture::InitVideoDevice Success.");
	
	return TRUE;
}
//Capture->GrabberSample->Render
BOOL CVideoCapture::ConnectPins()
{
	IPin *pCout;
	IPin *pGIn, *pGout;
	IPin *pRIn;

	pCout = FindPin(m_pCaptureFilter, PINDIR_OUTPUT);

	pGIn = FindPin(m_pGrabberFilter, PINDIR_INPUT);
	
	pGout = FindPin(m_pGrabberFilter, PINDIR_OUTPUT);

	pRIn = FindPin(m_pNullRenderFilter, PINDIR_INPUT);

	HRESULT hr = m_pGraphBuilder->ConnectDirect(pCout, pGIn, nullptr);
	if (FAILED(hr))
	{
		TRACE(" CVideoCapture::ConnectPins pCout, pGIn Failed.");
		return FALSE;
	}
	
	hr = m_pGraphBuilder->ConnectDirect(pGout, pRIn, nullptr);
	if (FAILED(hr))
	{
		TRACE(" CVideoCapture::ConnectPinspGout, pRIn Failed.");
		return FALSE;
	}

	return TRUE;
}
//查找Pin脚
IPin *CVideoCapture::FindPin(IBaseFilter *in_pFilter, PIN_DIRECTION in_dir)
{
	IEnumPins *pEnumPins = nullptr;
	IPin *pOutPin = nullptr;
	PIN_DIRECTION pDir;

	in_pFilter->EnumPins(&pEnumPins);
	
	while (pEnumPins->Next(1, &pOutPin, NULL) == S_OK)
	{
		pOutPin->QueryDirection(&pDir);
		if (pDir == in_dir)
		{
			return pOutPin;
		}
	}
	return nullptr;
}
//
IBaseFilter	*CVideoCapture::GetSpecifiedCaptureFilter(char *in_chFilterName)
{
	if (!m_pEnumMoniker)
		return nullptr;
	
	IMoniker	*pMoniker = nullptr;
	IBaseFilter *pFilter = nullptr;

	ULONG		ulFected = 0;
	
	HRESULT		hr = S_OK;
	
	while (m_pEnumMoniker->Next(1, &pMoniker, &ulFected) == S_OK)
	{
		IPropertyBag	*pProBag = nullptr;

		hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void **)&pProBag);
		if (SUCCEEDED(hr))
		{
			VARIANT	varName;

			VariantInit(&varName);

			hr = pProBag->Read(L"FriendlyName", &varName, 0);
			if (SUCCEEDED(hr))
			{
				char *pDest = WCharConvChar(varName.bstrVal);

				if (strcmp(in_chFilterName, pDest) == 0)
				{
					pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void **)&pFilter);
				}
				DEF_SAFE_DELETE_BUF(pDest);
			}
			VariantClear(&varName);
			pProBag->Release();
		}
		pMoniker->Release();
	}

	return pFilter;
}

//获取设备列表
BOOL CVideoCapture::GetDeviceList(char *out_chDevList[])
{
	if (!m_pEnumMoniker)
		return FALSE;

	IMoniker	*pMoniker = nullptr;
	ULONG		ulFected = 0;
	HRESULT		hr = S_OK;
	int			iCount = 0;

	while (m_pEnumMoniker->Next(1, &pMoniker, &ulFected) == S_OK)
	{
		IPropertyBag	*pProBag = nullptr;

		hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void **)&pProBag);
		if (SUCCEEDED(hr))
		{
			VARIANT	varName;
			
			VariantInit(&varName);
			
			hr = pProBag->Read(L"FriendlyName", &varName, 0);
			if (SUCCEEDED(hr))
			{
				char *pCh = WCharConvChar(varName.bstrVal);
				m_vecDevList.push_back(pCh);
				
				strcpy_s(out_chDevList[iCount],DEF_BUF_MAX,pCh);
				iCount++;
			}
			VariantClear(&varName);
			pProBag->Release();
		}
		pMoniker->Release();
	}
	
	m_pEnumMoniker->Reset();

	return TRUE;
}
// Release the format block for a media type.

void CVideoCapture::_FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}
void CVideoCapture::_DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
	if (pmt != NULL)
	{
		_FreeMediaType(*pmt);
		CoTaskMemFree(pmt);
	}
}
/*
获取设备支持的采集类型. 
*/
HRESULT CVideoCapture::GetVideoParams()
{
	HRESULT hr = S_OK;

	if (!m_pCaptureGraph)
	{
		TRACE(" CVideoCapture::GetVideoParams m_pCaptureGraph is null.");
		return E_POINTER;
	}
	//获取Config，第一个参数需要为PIN_CATEGORY_CAPTURE，如果为Privew则失败.
	hr = m_pCaptureGraph->FindInterface(&PIN_CATEGORY_CAPTURE,
						&MEDIATYPE_Video,
						m_pCaptureFilter,
						IID_IAMStreamConfig,
						(void **)&m_pConfig
					);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::GetVideoParams m_pCaptureGraph->FindInterface Failed.");
		return hr;
	}
	
	int iSize = 0, iCount = 0;

	hr = m_pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::GetVideoParams GetNumberOfCapabilities Failed.");
		return hr;
	}

	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		for (int iFormat = 0; iFormat < iCount; iFormat++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE *amt = nullptr;

			hr = m_pConfig->GetStreamCaps(iFormat, &amt, (BYTE *)&scc);
			if (SUCCEEDED(hr))
			{
				if (amt->formattype == FORMAT_VideoInfo)
				{
					VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)amt->pbFormat;
					TRACE("H:%d,W:%d,B:%d,F:%d\n", pvi->bmiHeader.biHeight, pvi->bmiHeader.biWidth, pvi->bmiHeader.biBitCount, pvi->AvgTimePerFrame);

					m_iBitRate = 32;
					m_iHeigh = pvi->bmiHeader.biHeight;
					m_iWidth = pvi->bmiHeader.biWidth;

					hr = m_pConfig->SetFormat(amt);

					_DeleteMediaType(amt);
					break;

				}

				_DeleteMediaType(amt);
			}
		}
	}
	return hr;
}
HRESULT CVideoCapture::SetWindowDisplay()
{
	if (m_pVideoWindow == nullptr)
	{
		TRACE(L"CVideoCapture::SetWindowDisplay m_pVideoWindow is nullptr.");
		return E_POINTER;
	}
	RECT re;
	
	GetWindowRect(m_hwnd, &re);

	HRESULT hr = m_pVideoWindow->put_Owner((OAHWND)m_hwnd);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::SetWindowDisplay m_pVideoWindow->put_Owner Failed.");
		return hr;
	}
	m_pVideoWindow->put_WindowStyle(WS_CHILD);

	m_pVideoWindow->SetWindowPosition(0, 0, re.right - re.left, re.bottom - re.top);

	m_pVideoWindow->put_Visible(OATRUE);

	return S_OK;
}
//打开设备
BOOL CVideoCapture::OpenDevice(char *in_chDevName)
{
	m_pCaptureFilter = GetSpecifiedCaptureFilter(in_chDevName);
	if (m_pCaptureFilter == nullptr)
	{
		TRACE("CVideoCapture::OpenDevice m_pCaptureFilter is nullptr.");
		return FALSE;
	}
	
	HRESULT hr = m_pGraphBuilder->AddFilter(m_pCaptureFilter, L"Capture Filter");
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::OpenDevice m_pGraphBuilder->AddFilter Failed.");
		return FALSE;
	}
	//
	hr = GetVideoParams();
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::OpenDevice GetVideoParams Failed.");
		return FALSE;
	}

	hr = m_pCaptureGraph->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, m_pCaptureFilter, NULL, NULL);	
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::OpenDevice m_pCaptureGraph->RenderStream PIN_CATEGORY_PREVIEW Failed.");
		return FALSE;
	}
	
	hr = m_pCaptureGraph->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, m_pCaptureFilter, m_pGrabberFilter, m_pNullRenderFilter);
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::OpenDevice m_pCaptureGraph->RenderStream PIN_CATEGORY_CAPTURE Failed."); 
		return FALSE;
	}
	//window
	hr = SetWindowDisplay();
	if (FAILED(hr))
	{
		TRACE("CVideoCapture::OpenDevice SetWindowDisplay Faild.");
		return FALSE;
	}

	if (m_pSampleCallBack)
		m_pSampleCallBack->SetPictureInfo(m_iHeigh, m_iWidth, m_iBitRate);

	return TRUE;
}
BOOL CVideoCapture::Start()
{
	if (m_pMediaControl)
		m_pMediaControl->Run();
	
	return TRUE;
}
BOOL CVideoCapture::CloseDevice()
{
	if (!m_vecDevList.empty())
	{
		std::vector<char *>::iterator iter; 
		for (iter = m_vecDevList.begin(); iter != m_vecDevList.end(); iter++)
		{
			DEF_SAFE_DELETE_BUF(*iter);
		}
		m_vecDevList.clear();
	}
	
	DEF_SAFE_DELETE_COM(m_pEnumMoniker);
	DEF_SAFE_DELETE_COM(m_pDevEnum);
	DEF_SAFE_DELETE_COM(m_pCaptureGraph);
	DEF_SAFE_DELETE_COM(m_pGraphBuilder);
	DEF_SAFE_DELETE_COM(m_pSampleCallBack);
	DEF_SAFE_DELETE_COM(m_pCaptureFilter);
	DEF_SAFE_DELETE_COM(m_pGrabberFilter);
	DEF_SAFE_DELETE_COM(m_pNullRenderFilter);
	DEF_SAFE_DELETE_COM(m_pMediaControl);
	DEF_SAFE_DELETE_COM(m_pVideoWindow);
	DEF_SAFE_DELETE_OBJECT(m_pSampleCallBack);
	DEF_SAFE_DELETE_COM(m_pConfig);

	return TRUE;
}
char *CVideoCapture::WCharConvChar(WCHAR *in_wchStr)
{
	int nSize = wcslen(in_wchStr);

	int nDst = WideCharToMultiByte(CP_ACP, 0, in_wchStr, nSize, NULL, 0, 0, 0);

	char *pDst = new char[nDst + 1];
	memset(pDst, 0, nDst + 1);

	WideCharToMultiByte(CP_ACP, 0, in_wchStr, nSize, pDst, nDst + 1, 0, 0);

	return pDst;
}