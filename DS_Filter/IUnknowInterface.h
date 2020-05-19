#pragma once
#include<dshow.h>
#pragma comment(lib,"Strmiids.lib")

DECLARE_INTERFACE(INonDelegatingUnknown)
{
	STDMETHOD(NonDelegatingQueryInterface) (THIS_ REFIID, LPVOID *) PURE;
	STDMETHOD_(ULONG, NonDelegatingAddRef)(THIS) PURE;
	STDMETHOD_(ULONG, NonDelegatingRelease)(THIS) PURE;
};
