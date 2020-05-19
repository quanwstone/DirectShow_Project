#include "Factory.h"



CFactory::CFactory(CLSID iid):m_clsid(iid)
{
	m_RefCount = 1;
	m_pFilter2 = nullptr;
}


CFactory::~CFactory()
{
}

HRESULT CFactory::QueryInterface(REFIID riid, LPVOID * pv)
{
	if (riid == IID_IClassFactory || riid == IID_IUnknown)
	{
		*pv =  static_cast<IClassFactory *>(this);
		AddRef();
		return S_OK;
	}
	return E_NOTIMPL;
}

ULONG CFactory::AddRef(void)
{
	return InterlockedIncrement(&m_RefCount);
}
ULONG CFactory::Release(void)
{
	LONG cnt = InterlockedDecrement(&m_RefCount);
	if (cnt == 0) {
		delete this;
	}
	return cnt;
}
HRESULT CFactory::CreateInstance(IUnknown * pUnk, REFIID riid, LPVOID * ppv)
{
	//m_pFilter = new CCBaseFilter(m_clsid);
	//if (m_pFilter->QueryInterface(riid, ppv)!=S_OK)
	//{
	//	delete m_pFilter;
	//	return E_NOINTERFACE;
	//}
	m_pFilter2 = new CCFilter(pUnk, m_clsid);
	if (m_pFilter2 != nullptr)
	{
		if (m_pFilter2->QueryInterface(riid, ppv) != S_OK)
		{
			delete m_pFilter2;
			return E_NOINTERFACE;
		}
	}
	return S_OK;
}

HRESULT CFactory::LockServer(BOOL fLock)
{
	return E_NOTIMPL;
}
