#pragma once
#include"CFilter.h"

class CFactory :public IClassFactory
{
public:
	explicit CFactory(CLSID m_clsd);
	~CFactory();
private:
	CLSID m_clsid;
	LONG m_RefCount;
	CCFilter *m_pFilter2;
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID *pv)override;
	
	virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
	
	virtual ULONG STDMETHODCALLTYPE Release(void)override;

	virtual HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnk, REFIID riid, LPVOID* ppv) override;
	
	virtual HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock)override;

};

