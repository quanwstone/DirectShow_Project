#pragma once
#include"CPin_D.h"

class CCLOCK {
public:
	CCLOCK(CRITICAL_SECTION *cs) {
		m_cs = cs;
		EnterCriticalSection(cs);
	};
	~CCLOCK() {
		LeaveCriticalSection(m_cs);
	}
private:
	CRITICAL_SECTION *m_cs;
};

class CCFilter :public INonDelegatingUnknown,
	public IBaseFilter, public IAMovieSetup
{
public:
	 CCFilter(IUnknown *punk,CLSID clsid);
	~CCFilter();
public:
	int				m_AllPinsCount;
	CCPin_D			**m_AllPins;
	PWCHAR			m_pFilterName;
	FILTER_STATE    m_FilterState;
	CRITICAL_SECTION m_cs;
	IReferenceClock* m_pClock;
private:
	LONG			m_RefCount;
	IUnknown*       m_pUnk;
	CLSID			m_clsid;
	CCPin_D*		m_pPin;
	IFilterGraph	*m_pGraph;
public:
	////INonDelegatingUnknown
	virtual HRESULT STDMETHODCALLTYPE NonDelegatingQueryInterface(
		REFIID riid,
		__RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)override;

	virtual ULONG STDMETHODCALLTYPE NonDelegatingAddRef(void)override;

	virtual ULONG STDMETHODCALLTYPE NonDelegatingRelease(void)override;

	/// IUnknown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		REFIID riid,
		__RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)override;

	virtual ULONG STDMETHODCALLTYPE AddRef(void)override;

	virtual ULONG STDMETHODCALLTYPE Release(void) override;

	////IPersist
	virtual HRESULT STDMETHODCALLTYPE GetClassID(
		__RPC__out CLSID *pClassID)override;

	////IMediaFilter
	virtual HRESULT STDMETHODCALLTYPE Stop(void)override;

	virtual HRESULT STDMETHODCALLTYPE Pause(void)override;

	virtual HRESULT STDMETHODCALLTYPE Run(REFERENCE_TIME tStart)override;

	virtual HRESULT STDMETHODCALLTYPE GetState(
		DWORD dwMilliSecsTimeout,
		__out  FILTER_STATE *State)override;

	virtual HRESULT STDMETHODCALLTYPE SetSyncSource(
		__in_opt  IReferenceClock *pClock)override;

	virtual HRESULT STDMETHODCALLTYPE GetSyncSource(
		__deref_out_opt  IReferenceClock **pClock)override;

	/////IBaseFilter
	virtual HRESULT STDMETHODCALLTYPE EnumPins(
		__out  IEnumPins **ppEnum)override;

	virtual HRESULT STDMETHODCALLTYPE FindPin(
		LPCWSTR Id,
		__out  IPin **ppPin)override;

	virtual HRESULT STDMETHODCALLTYPE QueryFilterInfo(
		__out  FILTER_INFO *pInfo)override;

	virtual HRESULT STDMETHODCALLTYPE JoinFilterGraph(
		__in_opt  IFilterGraph *pGraph,
		__in_opt  LPCWSTR pName)override;

	virtual HRESULT STDMETHODCALLTYPE QueryVendorInfo(
		__out  LPWSTR *pVendorInfo)override;

	//////IAMovieSetup
	virtual HRESULT STDMETHODCALLTYPE Register(void)override;

	virtual HRESULT STDMETHODCALLTYPE Unregister(void)override;
};
class CCEnumPins :public IEnumPins
{
protected:
	LONG	m_RefCount;
	LONG	m_Position;
	CCFilter* m_Filter;
public:
	CCEnumPins(CCFilter* vcam, CCEnumPins* o) :m_Filter(vcam), m_Position(0), m_RefCount(1) {
		m_Filter->AddRef();

		if (o)
			m_Position = o->m_Position;
	}
	~CCEnumPins() {
		m_Filter->Release();
	}
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		REFIID riid,
		__RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject) {
		HRESULT hr = S_OK;

		if (!ppvObject)
			return E_POINTER;

		if (riid == IID_IUnknown || riid == IID_IEnumPins) {
			*ppvObject = (IEnumPins*)(this);
		}
		else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}

		AddRef();
		return hr;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef(void) {
		return InterlockedIncrement(&m_RefCount);
	}
	virtual ULONG STDMETHODCALLTYPE Release(void) {
		LONG cnt = InterlockedDecrement(&m_RefCount);
		if (cnt == 0) {
			delete this;
		}
		return cnt;
	}
	//////
	STDMETHODIMP Next(
		ULONG cPins,         // place this many pins...
		__out_ecount(cPins) IPin ** ppPins,    // ...in this array of IPin*
		__out_opt ULONG * pcFetched    // actual count passed returned here
	)
	{
		HRESULT hr = S_OK;
		if (pcFetched != NULL) {
			*pcFetched = 0;           // default unless we succeed
		}
		else if (cPins > 1)
		{
			return E_INVALIDARG;
		}

		ULONG cFetched = 0;

		while (cPins) {

			if (m_Position >= m_Filter->m_AllPinsCount)
				break;
			*ppPins = m_Filter->m_AllPins[m_Position++];
			if (!*ppPins)
				break;
			(*ppPins)->AddRef();

			ppPins++;
			cFetched++;
			cPins--;
		}
		if (pcFetched != NULL) {
			*pcFetched = cFetched;
		}

		return (cPins == 0 ? S_OK : S_FALSE);
	}

	STDMETHODIMP Skip(ULONG cPins) {
		if (cPins == 0)
			return S_OK;

		m_Position += cPins;

		if (m_Position >= m_Filter->m_AllPinsCount)
			return E_UNEXPECTED;

		return S_OK;
	}
	STDMETHODIMP Reset() { m_Position = 0; return S_OK; }

	STDMETHODIMP Clone(__deref_out IEnumPins **ppEnum) {
		HRESULT hr = S_OK;
		if (!ppEnum)return E_POINTER;

		*ppEnum = new CCEnumPins(m_Filter, this);
		if (!*ppEnum)return E_OUTOFMEMORY;
		return hr;
	}
};