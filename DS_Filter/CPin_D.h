#pragma once
#include"IUnknowInterface.h"
#include"CConfig.h"
#include<thread>

#define SAFE_RELEASE(A)  if(A){ (A)->Release(); (A)=NULL; }
class CCFilter;

class CCPin_D:public INonDelegatingUnknown,public IPin, public IQualityControl, public IAMStreamConfig, public IKsPropertySet
{
public:
	explicit CCPin_D(CCFilter *p);
	~CCPin_D();
private:
	IUnknown		*m_pUnk;
	CCFilter		*m_pFilter;
	AM_MEDIA_TYPE	m_MediaType;
	IMemInputPin	*m_pInputPin;
	IPin			*m_pConnectPin;
	IMemAllocator	*m_pAllocator;

	HRESULT InternalConnect(IPin *pPin,AM_MEDIA_TYPE *pm,bool bConnect);

	HRESULT InternalEqualMediaType(AM_MEDIA_TYPE*a, AM_MEDIA_TYPE* b);

	void InternalDeleteMediaType(AM_MEDIA_TYPE *pmt);

	void InternalFreeMediaType(AM_MEDIA_TYPE& mt);

	HRESULT InternalCopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource);

	HRESULT InternalAllocMemory();

	HRESULT InternalAllocSize(IMemAllocator* pAlloc, ALLOCATOR_PROPERTIES* prop, AM_MEDIA_TYPE* mt);

public:
	PWCHAR				m_pPinName;
	AM_MEDIA_TYPE**		m_AllSupportedMediaTypes;
	LONG				m_AllSupportedMediaTypesCount;
	std::thread			m_thread;

	AM_MEDIA_TYPE *InternalCreateMediaType(AM_MEDIA_TYPE const *pSrc);
	
	bool	thread_capture();

	HRESULT Active(bool bActive);

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
	
	virtual ULONG STDMETHODCALLTYPE Release(void)override;

	////IPin
	virtual HRESULT STDMETHODCALLTYPE Connect(
		/* [in] */ IPin *pReceivePin,
		__in_opt  const AM_MEDIA_TYPE *pmt)override;

	virtual HRESULT STDMETHODCALLTYPE ReceiveConnection(
		/* [in] */ IPin *pConnector,
		/* [in] */ const AM_MEDIA_TYPE *pmt)override;
	
	virtual HRESULT STDMETHODCALLTYPE Disconnect(void)override;
	
	virtual HRESULT STDMETHODCALLTYPE ConnectedTo(__out  IPin **pPin)override;

	virtual HRESULT STDMETHODCALLTYPE ConnectionMediaType(__out  AM_MEDIA_TYPE *pmt)override;

	virtual HRESULT STDMETHODCALLTYPE QueryPinInfo(__out  PIN_INFO *pInfo)override;

	virtual HRESULT STDMETHODCALLTYPE QueryDirection(__out  PIN_DIRECTION *pPinDir)override;

	virtual HRESULT STDMETHODCALLTYPE QueryId(__out  LPWSTR *Id)override;

	virtual HRESULT STDMETHODCALLTYPE QueryAccept(/* [in] */ const AM_MEDIA_TYPE *pmt)override;

	virtual HRESULT STDMETHODCALLTYPE EnumMediaTypes(__out  IEnumMediaTypes **ppEnum)override;

	virtual HRESULT STDMETHODCALLTYPE QueryInternalConnections(
		__out_ecount_part_opt(*nPin, *nPin)  IPin **apPin,
		/* [out][in] */ ULONG *nPin)override;

	virtual HRESULT STDMETHODCALLTYPE EndOfStream(void)override;

	virtual HRESULT STDMETHODCALLTYPE BeginFlush(void)override;

	virtual HRESULT STDMETHODCALLTYPE EndFlush(void)override;

	virtual HRESULT STDMETHODCALLTYPE NewSegment(
		/* [in] */ REFERENCE_TIME tStart,
		/* [in] */ REFERENCE_TIME tStop,
		/* [in] */ double dRate)override;

	/////IQualityControl
	virtual HRESULT STDMETHODCALLTYPE Notify(
		/* [in] */ IBaseFilter *pSelf,
		/* [in] */ Quality q)override;

	virtual HRESULT STDMETHODCALLTYPE SetSink(
		/* [in] */ IQualityControl *piqc)override;

	////IAMStreamConfig
	virtual HRESULT STDMETHODCALLTYPE SetFormat(
		/* [in] */ AM_MEDIA_TYPE *pmt)override;

	virtual HRESULT STDMETHODCALLTYPE GetFormat(
		__out  AM_MEDIA_TYPE **ppmt)override;

	virtual HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(
		__out  int *piCount,
		__out  int *piSize)override;

	virtual HRESULT STDMETHODCALLTYPE GetStreamCaps(
		/* [in] */ int iIndex,
		__out  AM_MEDIA_TYPE **ppmt,
		__out  BYTE *pSCC)override;

	//////IKsPropertySet
	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Set(
		/* [in] */ REFGUID guidPropSet,
		/* [in] */ DWORD dwPropID,
		/* [annotation][size_is][in] */
		__in_bcount(cbInstanceData)  LPVOID pInstanceData,
		/* [in] */ DWORD cbInstanceData,
		/* [annotation][size_is][in] */
		__in_bcount(cbPropData)  LPVOID pPropData,
		/* [in] */ DWORD cbPropData)override;

	virtual /* [local] */ HRESULT STDMETHODCALLTYPE Get(
		/* [in] */ REFGUID guidPropSet,
		/* [in] */ DWORD dwPropID,
		/* [annotation][size_is][in] */
		__in_bcount(cbInstanceData)  LPVOID pInstanceData,
		/* [in] */ DWORD cbInstanceData,
		/* [annotation][size_is][out] */
		__out_bcount_part(cbPropData, *pcbReturned)  LPVOID pPropData,
		/* [in] */ DWORD cbPropData,
		/* [annotation][out] */
		__out  DWORD *pcbReturned)override;

	virtual HRESULT STDMETHODCALLTYPE QuerySupported(
		/* [in] */ REFGUID guidPropSet,
		/* [in] */ DWORD dwPropID,
		/* [annotation][out] */
		__out  DWORD *pTypeSupport)override;
};

class CEnumMediaTypes :public IEnumMediaTypes
{
	friend class CCPin_D;
public:
	CEnumMediaTypes(CCPin_D *p, CEnumMediaTypes *pEnum) {

		m_pPin = p;
		m_pPin->AddRef();

		m_Position = 0;
		if (pEnum != nullptr)
		{
			m_Position = pEnum->m_Position;
		}
		m_RefCount = 1;
	}
	~CEnumMediaTypes()
	{
		SAFE_RELEASE(m_pPin);
	}
private:
	CCPin_D *m_pPin;
	LONG m_RefCount;
	LONG m_Position;
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		REFIID riid,
		__RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		HRESULT hr = S_OK;

		if (!ppvObject)
			return E_POINTER;

		if (riid == IID_IUnknown || riid == IID_IEnumMediaTypes) {
			*ppvObject = (IEnumMediaTypes*)(this);
		}
		else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
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
	////
	virtual HRESULT STDMETHODCALLTYPE Next(
		ULONG cMediaTypes,
		__out_ecount_part(cMediaTypes, *pcFetched)  AM_MEDIA_TYPE **ppMediaTypes,
		__out_opt  ULONG *pcFetched)
	{
		HRESULT hr = S_OK;

		if (pcFetched != NULL) {
			*pcFetched = 0;           // default unless we succeed
		}
		else if (cMediaTypes > 1)
		{
			return E_INVALIDARG;
		}

		ULONG cFetched = 0;

		while (cMediaTypes) {
			//
			if (m_Position >= m_pPin->m_AllSupportedMediaTypesCount)
				break;
			*ppMediaTypes = m_pPin->InternalCreateMediaType(m_pPin->m_AllSupportedMediaTypes[m_Position++]);
			if (!*ppMediaTypes)break;

			////
			ppMediaTypes++;
			cFetched++;
			cMediaTypes--;
		}
		if (pcFetched != NULL) {
			*pcFetched = cFetched;
		}

		return (cMediaTypes == 0 ? NOERROR : S_FALSE);
	}
	virtual HRESULT STDMETHODCALLTYPE Skip(/* [in] */ ULONG cMediaTypes) {
		if (cMediaTypes == 0)
			return S_OK;

		m_Position += cMediaTypes;

		if (m_Position >= m_pPin->m_AllSupportedMediaTypesCount)
			return E_UNEXPECTED;

		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE Reset(void) { m_Position = 0; return S_OK; }

	virtual HRESULT STDMETHODCALLTYPE Clone(__out  IEnumMediaTypes **ppEnum)
	{
		HRESULT hr = S_OK;

		if (!ppEnum)
			return E_POINTER;

		*ppEnum = new CEnumMediaTypes(m_pPin, this);

		if (!*ppEnum)
			return E_OUTOFMEMORY;
		return hr;
	}
};