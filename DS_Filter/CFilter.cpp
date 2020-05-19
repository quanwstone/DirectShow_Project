#include "CFilter.h"



CCFilter::CCFilter(IUnknown *punk,CLSID clsid):m_clsid(clsid)
{
	if (punk != nullptr)
	{
		m_pUnk = punk;
	}
	else {
		m_pUnk = (IUnknown*)static_cast<INonDelegatingUnknown*>(this);
	}
	InitializeCriticalSection(&m_cs);
	m_FilterState = State_Stopped;
	m_RefCount = 0;

	m_pClock = nullptr;
	m_pFilterName = nullptr;
	m_pGraph = nullptr;

	m_pPin = nullptr;
	m_pPin = new CCPin_D(this);
	
	m_AllPinsCount = 1;
	m_AllPins = new CCPin_D *[m_AllPinsCount];
	m_AllPins[0] = m_pPin;
}


CCFilter::~CCFilter()
{
	DeleteCriticalSection(&m_cs);
}

HRESULT CCFilter::NonDelegatingQueryInterface(REFIID riid, void ** ppvObject)
{
	HRESULT hr = S_OK;

	if (!ppvObject)
		return E_POINTER;
	
	*ppvObject = NULL;
	
	if (riid == IID_IAMStreamConfig || riid == IID_IKsPropertySet) {
		return m_pPin->QueryInterface(riid, ppvObject);
	}
	if (riid == IID_IUnknown) {
		*ppvObject = static_cast<INonDelegatingUnknown*>(this);
	}
	else if (riid == IID_IBaseFilter) {
		*ppvObject = static_cast<IBaseFilter*>(this);
	}
	else if (riid == IID_IPersist) {
		*ppvObject = (IPersist*)this;
	}
	else if (riid == IID_IMediaFilter) {
		*ppvObject = (IMediaFilter*)this;
	}
	else if (riid == IID_IAMovieSetup) {
		*ppvObject = (IAMovieSetup*)this;
	}
	else {
		*ppvObject = 0;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG CCFilter::NonDelegatingAddRef(void)
{
	return InterlockedIncrement(&m_RefCount);
}

ULONG CCFilter::NonDelegatingRelease(void)
{
	LONG cnt = InterlockedDecrement(&m_RefCount);
	if (cnt == 0) {
		delete this;
	}
	return cnt;
}

HRESULT CCFilter::QueryInterface(REFIID riid, void ** ppvObject)
{
	return m_pUnk->QueryInterface(riid,ppvObject);
}

ULONG CCFilter::AddRef(void)
{
	return m_pUnk->AddRef();
}

ULONG CCFilter::Release(void)
{
	return m_pUnk->Release();
}

HRESULT CCFilter::GetClassID(CLSID * pClassID)
{
	if (!pClassID)
		return E_POINTER;

	*pClassID = m_clsid;
	return S_OK;
}

HRESULT CCFilter::Stop(void)
{
	HRESULT hr = S_OK;  ///
	
	OutputDebugString("CCFilter::Stop\n");

	CCLOCK cs(&m_cs);
	if (m_FilterState != State_Stopped) {
		m_pPin->Active(FALSE);	
	}
	m_FilterState = State_Stopped;

	return S_OK;
}

HRESULT CCFilter::Pause(void)
{
	HRESULT hr = S_OK;
	OutputDebugString("CCFilter::Pause\n");

	CCLOCK cs(&m_cs);
	if (m_FilterState == State_Stopped) {	
		hr = m_pPin->Active(TRUE);	
	}

	m_FilterState = State_Paused;
	
	return S_OK;
}

HRESULT CCFilter::Run(REFERENCE_TIME tStart)
{
	HRESULT hr = S_OK;
	OutputDebugString("CCFilter:;Run\n");

	CCLOCK cs(&m_cs);
	
	m_FilterState = State_Running;
	
	hr = m_pPin->Active(TRUE);
	
	return hr;
}

HRESULT CCFilter::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE * State)
{
	OutputDebugString("CCFilter::GetState\r\n");

	if (!State)
		return E_POINTER;
	CCLOCK cs(&m_cs);

	*State = m_FilterState;
	
	return S_OK;
}

HRESULT CCFilter::SetSyncSource(IReferenceClock * pClock)
{
	OutputDebugString("CCFilter::SetSyncSource\r\n");
	
	CCLOCK cs(&m_cs);
	m_pClock = pClock;
	
	if(m_pClock != nullptr)
		m_pClock->AddRef();

	return S_OK;
}

HRESULT CCFilter::GetSyncSource(IReferenceClock ** pClock)
{
	OutputDebugString("CCFilter::GetSyncSource\r\n");
	CCLOCK cs(&m_cs);
	
	*pClock = m_pClock;
	m_pClock->AddRef();

	return S_OK;
}

HRESULT CCFilter::EnumPins(IEnumPins ** ppEnum)
{
	OutputDebugString("CCFilter::EnumPins\r\n");

	if (!ppEnum)
		return E_POINTER;

	*ppEnum = new CCEnumPins(this, NULL);

	if (!*ppEnum)
		return E_OUTOFMEMORY;
	
	return S_OK;
}

HRESULT CCFilter::FindPin(LPCWSTR Id, IPin ** ppPin)
{
	OutputDebugString("CCFilter::FindPin\r\n");

	if (!Id || !ppPin) 
	{
		return E_POINTER;
	}

	for (int i = 0; i < m_AllPinsCount; ++i) 
	{
		if (wcscmp(m_AllPins[i]->m_pPinName, Id) == 0) 
		{
			*ppPin = m_AllPins[i];
			(*ppPin)->AddRef();
			return S_OK;
		}
	}
	*ppPin = NULL;
	return VFW_E_NOT_FOUND;
}

HRESULT CCFilter::QueryFilterInfo(FILTER_INFO * pInfo)
{
	OutputDebugString("CCFilter::QueryFilterInfo\r\n");

	if (!pInfo)
		return E_POINTER;
	
	CCLOCK cs(&m_cs);
	if (m_pFilterName) {
		wcscpy(pInfo->achName, m_pFilterName);
	}
	else {
		pInfo->achName[0] = 0;
	}
	
	pInfo->pGraph = m_pGraph;
	
	if (pInfo->pGraph)
		pInfo->pGraph->AddRef();

	return S_OK;
}

HRESULT CCFilter::JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName)
{
	OutputDebugString("CCFilter::JoinFilterGraph\r\n");

	CCLOCK cs(&m_cs);
	
	m_pGraph = pGraph;

	if (m_pFilterName) {
		free(m_pFilterName);
		m_pFilterName = NULL;
	}
	if (pName) {
		m_pFilterName = (wchar_t*)malloc(sizeof(wchar_t)*(wcslen(pName) + 1));
		wcscpy(m_pFilterName, pName);
	}
	return S_OK;
}

HRESULT CCFilter::QueryVendorInfo(LPWSTR * pVendorInfo)
{
	OutputDebugString("CCFilter::QueryVendorInfo\r\n");

	return E_NOTIMPL;
}

HRESULT CCFilter::Register(void)
{
	OutputDebugString("CCFilter::Register\r\n");

	return E_NOTIMPL;
}

HRESULT CCFilter::Unregister(void)
{
	OutputDebugString("CCFilter::Unregister\r\n");

	return E_NOTIMPL;
}
