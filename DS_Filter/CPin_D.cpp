#include "CPin_D.h"
#include"CFilter.h"

CCPin_D::CCPin_D(CCFilter *p)
{
	m_pFilter = p;
	m_pFilter->AddRef();
	m_pPinName = L"VCamera OutputPin";
	m_AllSupportedMediaTypesCount = 1;
	m_AllSupportedMediaTypes = VCamAllMediaTypes;

	m_pInputPin = nullptr;
	m_pAllocator = nullptr;
	m_pConnectPin = nullptr;
	
	InternalCopyMediaType(&m_MediaType, const_cast<AM_MEDIA_TYPE*>(VCamAllMediaTypes[0]));

	m_pUnk = (IUnknown*)static_cast<INonDelegatingUnknown*>(this);

	m_thread = std::thread(&CCPin_D::thread_capture, this);
}


CCPin_D::~CCPin_D()
{
}

HRESULT CCPin_D::InternalConnect(IPin * pPin, AM_MEDIA_TYPE * pm, bool bConnect)
{
	HRESULT hr = E_NOTIMPL;
	PIN_DIRECTION dir;
	
	hr = pPin->QueryDirection(&dir);
	if (hr == S_OK)
	{
		if (dir == PINDIR_OUTPUT)
		{
			return VFW_E_INVALID_DIRECTION;
		}
	}
	else {
		return hr;
	}

	hr = pPin->QueryInterface(IID_IMemInputPin, (void**)&m_pInputPin);
	if (SUCCEEDED(hr))
	{
		hr = InternalEqualMediaType(&m_MediaType, pm);
		if (hr == S_OK)
		{
			m_pConnectPin = pPin;
			m_pConnectPin->AddRef();
				
			InternalFreeMediaType(m_MediaType);
			InternalCopyMediaType(&m_MediaType, pm);
			
			if (bConnect)
			{
				hr = pPin->ReceiveConnection((IPin*)this, pm);
			}
		}
		if (hr == S_OK)
		{
			hr = InternalAllocMemory();
		}
	}

	if (hr != S_OK)
	{
		SAFE_RELEASE(m_pAllocator);
		SAFE_RELEASE(m_pInputPin);
		SAFE_RELEASE(m_pConnectPin);
	}
	return hr;
}

HRESULT CCPin_D::InternalEqualMediaType(AM_MEDIA_TYPE * a, AM_MEDIA_TYPE * b)
{
	if ((IsEqualGUID(a->majortype, b->majortype)) &&
		(IsEqualGUID(a->subtype, b->subtype)) &&
		(IsEqualGUID(a->formattype, b->formattype)) &&
		(a->cbFormat == b->cbFormat))
	{
		if (a->cbFormat == 0)
			return S_OK;
		
		VIDEOINFOHEADER* p1 = (VIDEOINFOHEADER*)a->pbFormat;
		VIDEOINFOHEADER* p2 = (VIDEOINFOHEADER*)b->pbFormat;
		if (p1->bmiHeader.biWidth == p2->bmiHeader.biWidth && p1->bmiHeader.biHeight == p2->bmiHeader.biHeight
			&& p1->bmiHeader.biBitCount == p2->bmiHeader.biBitCount)
			return S_OK;
	}
	return S_FALSE;
}

void CCPin_D::InternalDeleteMediaType(AM_MEDIA_TYPE * pmt)
{
	if (pmt == NULL) {
		return;
	}

	InternalFreeMediaType(*pmt);
	CoTaskMemFree((PVOID)pmt);
}

void CCPin_D::InternalFreeMediaType(AM_MEDIA_TYPE & mt)
{
	if (mt.cbFormat != 0) {
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL) {
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

AM_MEDIA_TYPE * CCPin_D::InternalCreateMediaType(AM_MEDIA_TYPE const * pSrc)
{
	AM_MEDIA_TYPE *pMediaType =
		(AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));

	if (pMediaType == NULL) {
		return NULL;
	}

	HRESULT hr = InternalCopyMediaType(pMediaType, pSrc);
	if (FAILED(hr)) {
		CoTaskMemFree((PVOID)pMediaType);
		return NULL;
	}

	return pMediaType;
}

bool CCPin_D::thread_capture()
{
	FILE *pSrc = nullptr;
	fopen_s(&pSrc, "C:\\quanwei\\WMV\\264\\1280422.yuv", "rb");
	
	const int len = 1280 * 720 * 2;//yuv422
	char *buf = new char[len];
	memset(buf, 0, len);

	while (true)
	{
		Sleep(30);
		
		if (m_pFilter->m_FilterState != State_Running)
		{
			continue;
		}
		int ir = fread(buf, 1, len, pSrc);
		if (ir > 0)
		{
			IMediaSample* sample = NULL;
			BYTE *pbuf = nullptr;
			int iSize = 0;
			HRESULT hr = S_FALSE;
			
			{
				CCLOCK cs(&m_pFilter->m_cs);
				if (m_pAllocator) 
				{
					hr = m_pAllocator->GetBuffer(&sample, NULL, NULL, 0);
				}
			}
			if (hr == S_OK)
			{
				sample->GetPointer(&pbuf);
				iSize = sample->GetSize();
				memcpy(pbuf, buf, len);
				sample->SetSyncPoint(true);

				{
					CCLOCK cs(&m_pFilter->m_cs);
					if (m_pInputPin != nullptr)
					{
						hr = m_pInputPin->Receive(sample);
						if (hr != S_OK)
						{
							m_pConnectPin->EndOfStream();
						}
					}
				}
			}
			SAFE_RELEASE(sample);
		}
	}
	return false;
}

HRESULT CCPin_D::Active(bool bActive)
{
	HRESULT hr; 
	if (!m_pConnectPin || !m_pAllocator)
		return E_FAIL;

	if (bActive) 
		hr = m_pAllocator->Commit();
	else 
		hr = m_pAllocator->Decommit();
	
	return hr;	
}

HRESULT CCPin_D::InternalCopyMediaType(AM_MEDIA_TYPE * pmtTarget, const AM_MEDIA_TYPE * pmtSource)
{
	*pmtTarget = *pmtSource;

	if (pmtSource->cbFormat != 0) {

		pmtTarget->pbFormat = (PBYTE)CoTaskMemAlloc(pmtSource->cbFormat);
		if (pmtTarget->pbFormat == NULL) {
			pmtTarget->cbFormat = 0;
			return E_OUTOFMEMORY;
		}
		else {
			CopyMemory((PVOID)pmtTarget->pbFormat, (PVOID)pmtSource->pbFormat,
				pmtTarget->cbFormat);
		}
	}
	if (pmtTarget->pUnk != NULL) {
		pmtTarget->pUnk->AddRef();
	}

	return S_OK;
}

HRESULT CCPin_D::InternalAllocMemory()
{
	HRESULT hr = S_OK;

	ALLOCATOR_PROPERTIES prop;
	ZeroMemory(&prop, sizeof(prop));

	m_pInputPin->GetAllocatorRequirements(&prop);
	if (prop.cbAlign == 0) {
		prop.cbAlign = 1;
	}

	hr = m_pInputPin->GetAllocator(&m_pAllocator);
	if (SUCCEEDED(hr)) 
	{
		hr = InternalAllocSize(m_pAllocator, &prop, &m_MediaType);
		if (SUCCEEDED(hr)) {
			hr = m_pInputPin->NotifyAllocator(m_pAllocator, FALSE);
			if (SUCCEEDED(hr))
			{
				return S_OK;
			}
		}
	}
	SAFE_RELEASE(m_pAllocator);

	hr = CoCreateInstance(CLSID_MemoryAllocator, 0, CLSCTX_INPROC_SERVER, IID_IMemAllocator, (void **)&m_pAllocator);
	if (FAILED(hr)) return hr;

	hr = InternalAllocSize(m_pAllocator, &prop, &m_MediaType);
	if (SUCCEEDED(hr)) {
		hr = m_pInputPin->NotifyAllocator(m_pAllocator, FALSE);
		if (SUCCEEDED(hr)) return S_OK;
	}

	SAFE_RELEASE(m_pAllocator);

	return hr;
}

HRESULT CCPin_D::InternalAllocSize(IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * prop, AM_MEDIA_TYPE * mt)
{
	VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)mt->pbFormat;

	if (!pvi)return E_INVALIDARG;
	prop->cBuffers = 1;
	prop->cbAlign = 1;
	prop->cbBuffer = pvi->bmiHeader.biSizeImage;

	ALLOCATOR_PROPERTIES Actual; memset(&Actual, 0, sizeof(Actual));

	HRESULT hr = pAlloc->SetProperties(prop, &Actual);

	if (FAILED(hr)) 
		return hr;

	if (Actual.cbBuffer < prop->cbBuffer)
		return E_FAIL;
	
	return S_OK;
}

HRESULT CCPin_D::NonDelegatingQueryInterface(REFIID riid, void ** ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject)
		return E_POINTER;
	
	if (riid == IID_IUnknown) {
		*ppvObject = (INonDelegatingUnknown*)this;
	}
	else if (riid == IID_IPin) {
		*ppvObject = (IPin*)(this);
	}
	else if (riid == IID_IQualityControl) {
		*ppvObject = (IQualityControl*)this;
	}
	else if (riid == IID_IAMStreamConfig) {
		*ppvObject = (IAMStreamConfig*)this;
	}
	else if (riid == IID_IKsPropertySet) {
		*ppvObject = (IKsPropertySet*)this;
	}
	else {
		*ppvObject = 0;
		return E_NOINTERFACE;
	}
	AddRef();
	return S_OK;
}

ULONG CCPin_D::NonDelegatingAddRef(void)
{
	return m_pFilter->AddRef();
}

ULONG CCPin_D::NonDelegatingRelease(void)
{
	return m_pFilter->Release();
}

HRESULT CCPin_D::QueryInterface(REFIID riid, void ** ppvObject)
{
	return m_pUnk->QueryInterface(riid,ppvObject);
}

ULONG CCPin_D::AddRef(void)
{
	return m_pUnk->AddRef();
}

ULONG CCPin_D::Release(void)
{
	return m_pUnk->Release();;
}

HRESULT CCPin_D::Connect(IPin * pReceivePin, const AM_MEDIA_TYPE * pmt)
{
	OutputDebugString("CCPin_D::Connect\r\n");

	HRESULT hr = S_OK;

	CCLOCK cs(&m_pFilter->m_cs);
	
	if (!pReceivePin)
		return E_POINTER;

	if (m_pConnectPin)
	{
		return VFW_E_ALREADY_CONNECTED;
	}

	if (pmt && pmt->majortype != GUID_NULL && pmt->formattype != GUID_NULL) 
	{
	
		hr = InternalConnect(pReceivePin, const_cast<AM_MEDIA_TYPE*>(pmt), true);
		return hr;
	}

	IEnumMediaTypes* types = nullptr;
	AM_MEDIA_TYPE *mt = nullptr;
	ULONG ul_fetched = 0;
	
	hr = pReceivePin->EnumMediaTypes(&types);
	if (SUCCEEDED(hr))
	{
		types->Reset();
		while (types->Next(1, &mt, &ul_fetched)==S_OK)
		{
			VIDEOINFOHEADER* p1 = (VIDEOINFOHEADER*)m_MediaType.pbFormat;
			VIDEOINFOHEADER* p2 = (VIDEOINFOHEADER*)mt->pbFormat;

			hr = InternalConnect(pReceivePin, mt, true);
			InternalDeleteMediaType(mt);

			if (hr == S_OK)
			{
				SAFE_RELEASE(types);
				return hr;
			}
		}
	}

	SAFE_RELEASE(types);

	hr = EnumMediaTypes(&types);
	if (SUCCEEDED(hr))
	{
		types->Reset();
		while (types->Next(1, &mt, &ul_fetched) == S_OK)
		{
			VIDEOINFOHEADER* p1 = (VIDEOINFOHEADER*)m_MediaType.pbFormat;
			VIDEOINFOHEADER* p2 = (VIDEOINFOHEADER*)mt->pbFormat;

			hr = InternalConnect(pReceivePin, mt, true);
			InternalDeleteMediaType(mt);

			if (hr == S_OK)
			{
				SAFE_RELEASE(types);
				return hr;
			}
		}
	}

	return VFW_E_NO_ACCEPTABLE_TYPES;
}

HRESULT CCPin_D::ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE * pmt)
{
	OutputDebugString("CCPin_D::ReceiveConnection\r\n");

	HRESULT hr = S_OK;

	if (!pConnector || !pmt)
		return E_POINTER;
	
	CCLOCK cs(&m_pFilter->m_cs);

	if (m_pConnectPin) {
		return VFW_E_ALREADY_CONNECTED;
	}

	hr = InternalConnect(pConnector, const_cast<AM_MEDIA_TYPE*>(pmt), false);

	return hr;
}

HRESULT CCPin_D::Disconnect(void)
{
	OutputDebugString("CCPin_D::Disconnect\r\n");

	HRESULT hr = S_OK;
	////
	CCLOCK cs(&m_pFilter->m_cs);

	if (!m_pConnectPin) 
		return S_FALSE;

	if (m_pAllocator) {
		m_pAllocator->Decommit();
		SAFE_RELEASE(m_pAllocator);
	}
	SAFE_RELEASE(m_pInputPin);
	SAFE_RELEASE(m_pConnectPin);

	return S_OK;
}

HRESULT CCPin_D::ConnectedTo(IPin ** pPin)
{
	OutputDebugString("CCPin_D::ConnectedTo\r\n");

	if (!pPin)
		return E_POINTER;

	CCLOCK cs(&m_pFilter->m_cs);

	*pPin = m_pConnectPin;

	if (m_pConnectPin) {
		m_pConnectPin->AddRef();
		return S_OK;
	}
	return VFW_E_NOT_CONNECTED;
}

HRESULT CCPin_D::ConnectionMediaType(AM_MEDIA_TYPE * pmt)
{
	OutputDebugString("CCPin_D::ConnectionMediaType\r\n");

	return E_NOTIMPL;
}

HRESULT CCPin_D::QueryPinInfo(PIN_INFO * pInfo)
{
	OutputDebugString("CCPin_D::QueryPinInfo\r\n");

	if (!pInfo)
		return E_POINTER;

	pInfo->pFilter = (IBaseFilter*)m_pFilter;
	pInfo->pFilter->AddRef();
	/////
	if (m_pPinName) {
		wcscpy_s(pInfo->achName, m_pPinName);
	}
	else {
		pInfo->achName[0] = 0;
	}
	pInfo->dir = PINDIR_OUTPUT;

	return S_OK;
}

HRESULT CCPin_D::QueryDirection(PIN_DIRECTION * pPinDir)
{
	OutputDebugString("CCPin_D::QueryDirection\r\n");

	if (!pPinDir)
		return E_POINTER;
	
	*pPinDir = PINDIR_OUTPUT;

	return S_OK;
}

HRESULT CCPin_D::QueryId(LPWSTR * Id)
{
	OutputDebugString("CCPin_D::QueryId\r\n");

	if (!Id)
		return E_POINTER;

	if (!m_pPinName)
		return E_INVALIDARG;

	LONG Len = (wcslen(m_pPinName) + 1) * sizeof(WCHAR);

	*Id = (PWCHAR)CoTaskMemAlloc(Len);
	
	if (!*Id)
		return E_OUTOFMEMORY;
	
	wcscpy(*Id, m_pPinName);

	return S_OK;
}

HRESULT CCPin_D::QueryAccept(const AM_MEDIA_TYPE * pmt)
{
	OutputDebugString("CCPin_D::QueryAccept\r\n");

	if (!pmt)
		return E_POINTER;

	CCLOCK cs(&m_pFilter->m_cs);

	if (InternalEqualMediaType((AM_MEDIA_TYPE*)pmt, &m_MediaType)) 
		return S_OK;
	
	return S_FALSE;
}

HRESULT CCPin_D::EnumMediaTypes(IEnumMediaTypes ** ppEnum)
{
	OutputDebugString("CCPin_D::EnumMediaTypes\r\n");

	if (!ppEnum)return E_POINTER;

	*ppEnum = new CEnumMediaTypes(this, NULL);
	if (!*ppEnum)
		return E_OUTOFMEMORY;
	
	return S_OK;
}

HRESULT CCPin_D::QueryInternalConnections(IPin ** apPin, ULONG * nPin)
{
	OutputDebugString("CCPin_D::QueryInternalConnections\r\n");

	return E_NOTIMPL;
}

HRESULT CCPin_D::EndOfStream(void)
{
	OutputDebugString("CCPin_D::EndOfStream\r\n");

	return E_NOTIMPL;
}

HRESULT CCPin_D::BeginFlush(void)
{
	OutputDebugString("CCPin_D::BeginFlush\r\n");

	return E_NOTIMPL;
}

HRESULT CCPin_D::EndFlush(void)
{
	OutputDebugString("CCPin_D::EndFlush\r\n");

	return E_NOTIMPL;
}

HRESULT CCPin_D::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	OutputDebugString("CCPin_D::NewSegment\r\n");

	return E_NOTIMPL;
}

HRESULT CCPin_D::Notify(IBaseFilter * pSelf, Quality q)
{
	OutputDebugString("CCPin_D::Notify\r\n");

	return E_NOTIMPL;
}

HRESULT CCPin_D::SetSink(IQualityControl * piqc)
{
	OutputDebugString("CCPin_D::SetSink\r\n");

	return E_NOTIMPL;
}

HRESULT CCPin_D::SetFormat(AM_MEDIA_TYPE * pmt)
{
	OutputDebugString("CCPin_D::SetFormat\r\n");

	HRESULT hr = VFW_E_NOT_FOUND; //
	
	if (!pmt)
		return E_POINTER;
	
	CCLOCK cs(&m_pFilter->m_cs);

	for (int i = 0; i < m_AllSupportedMediaTypesCount; ++i) {
		if (InternalEqualMediaType(pmt, m_AllSupportedMediaTypes[i])==S_OK) {
			InternalFreeMediaType(m_MediaType);
			hr = InternalCopyMediaType(&m_MediaType, pmt);
			break;
		}
	}
	return hr;
}

HRESULT CCPin_D::GetFormat(AM_MEDIA_TYPE ** ppmt)
{
	OutputDebugString("CCPin_D::GetFormat\r\n");

	if (!ppmt)
		return E_POINTER;

	CCLOCK cs(&m_pFilter->m_cs);

	*ppmt = InternalCreateMediaType(&m_MediaType);

	if (!*ppmt)
		return E_OUTOFMEMORY;
	
	return S_OK;
}

HRESULT CCPin_D::GetNumberOfCapabilities(int * piCount, int * piSize)
{
	OutputDebugString("CCPin_D::GetNumberOfCapabilities\r\n");

	if (!piCount || !piSize)
		return E_POINTER;

	*piCount = m_AllSupportedMediaTypesCount;

	*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);

	return S_OK;
}

HRESULT CCPin_D::GetStreamCaps(int iIndex, AM_MEDIA_TYPE ** ppmt, BYTE * pSCC)
{
	OutputDebugString("CCPin_D::GetStreamCaps\r\n");

	if (!ppmt || !pSCC)
		return E_POINTER;

	if (iIndex < 0 || iIndex >= m_AllSupportedMediaTypesCount) {
		return E_INVALIDARG;
	}
	*ppmt = InternalCreateMediaType(m_AllSupportedMediaTypes[iIndex]);

	if (!*ppmt)
		return E_OUTOFMEMORY;

	/////
	VIDEO_STREAM_CONFIG_CAPS*pvscc = (VIDEO_STREAM_CONFIG_CAPS*)pSCC;
	VIDEOINFOHEADER* pvi = (VIDEOINFOHEADER*)(*ppmt)->pbFormat;

	int X = pvi->bmiHeader.biWidth;
	int Y = abs(pvi->bmiHeader.biHeight);

	pvscc->guid = FORMAT_VideoInfo;
	pvscc->VideoStandard = AnalogVideo_None;
	pvscc->InputSize.cx = X;
	pvscc->InputSize.cy = Y;
	pvscc->MinCroppingSize.cx = X;
	pvscc->MinCroppingSize.cy = Y;
	pvscc->MaxCroppingSize.cx = X;
	pvscc->MaxCroppingSize.cy = Y;
	pvscc->CropGranularityX = X;
	pvscc->CropGranularityY = Y;
	pvscc->CropAlignX = 0;
	pvscc->CropAlignY = 0;

	pvscc->MinOutputSize.cx = X;
	pvscc->MinOutputSize.cy = Y;
	pvscc->MaxOutputSize.cx = X;
	pvscc->MaxOutputSize.cy = Y;
	pvscc->OutputGranularityX = 0;
	pvscc->OutputGranularityY = 0;
	pvscc->StretchTapsX = 0;
	pvscc->StretchTapsY = 0;
	pvscc->ShrinkTapsX = 0;
	pvscc->ShrinkTapsY = 0;
	pvscc->MinFrameInterval = pvi->AvgTimePerFrame;   //50 fps
	pvscc->MaxFrameInterval = pvi->AvgTimePerFrame; // 0.2 fps
	pvscc->MinBitsPerSecond = (80 * 60 * 3 * 8) / 5;
	pvscc->MaxBitsPerSecond = X * Y * 3 * 8 * 50;

	return S_OK;
}

HRESULT CCPin_D::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
	OutputDebugString("CCPin_D::Set\r\n");

	return E_NOTIMPL;
}

HRESULT CCPin_D::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD * pcbReturned)
{
	OutputDebugString("CCPin_D::Get\r\n");

	if (guidPropSet != AMPROPSETID_Pin)             return E_PROP_SET_UNSUPPORTED;
	if (dwPropID != AMPROPERTY_PIN_CATEGORY)        return E_PROP_ID_UNSUPPORTED;
	if (pPropData == NULL && pcbReturned == NULL)   return E_POINTER;

	if (pcbReturned) *pcbReturned = sizeof(GUID);
	if (pPropData == NULL)          return S_OK; // Caller just wants to know the size. 
	if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;// The buffer is too small.

	*(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
	return S_OK;
}

HRESULT CCPin_D::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD * pTypeSupport)
{
	OutputDebugString("CCPin_D::QuerySupported\r\n");

	if (guidPropSet != AMPROPSETID_Pin)
		return E_PROP_SET_UNSUPPORTED;

	if (dwPropID != AMPROPERTY_PIN_CATEGORY) 
		return E_PROP_ID_UNSUPPORTED;

	if (pTypeSupport) 
		*pTypeSupport = KSPROPERTY_SUPPORT_GET;

	return S_OK;
}
