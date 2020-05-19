#pragma once
#include<mfapi.h>
#include<mfidl.h>
#include<mfreadwrite.h>
#include<wmsdkidl.h>

#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"mf.lib")
#pragma comment(lib,"Mfreadwrite.lib")
#pragma comment(lib,"mfuuid.lib")
#pragma comment(lib,"evr.lib")
class CCMediaFoundation
{
public:
	CCMediaFoundation();
	~CCMediaFoundation();
public:
	HRESULT InitMediaFoundation();
	HRESULT CreateVideoDeviceSource(IMFMediaSource **ppSource);
	HRESULT EnumDeviceMediaType(IMFMediaSource *pSource);
};

