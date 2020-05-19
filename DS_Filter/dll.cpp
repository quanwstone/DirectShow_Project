#include<Windows.h>
#include<string>
#include<stdio.h>
#include <atlbase.h>
#include"Factory.h"

//#include<dshow.h>
#pragma comment(lib,"Strmiids.lib")

HMODULE g_moduleinstall;

static const GUID CLSID_QWCamDShow =
{ 0x6547a25b, 0xa535, 0x4d13,{ 0x97, 0x5d, 0xec, 0x13, 0xd9, 0x76, 0x7e, 0xb0 } };


REGPINTYPES PinTypes = {
	&MEDIATYPE_Video,
	&MEDIASUBTYPE_NULL
};
REGFILTERPINS VCamPins = {
	L"Pins",
	FALSE, /// 
	TRUE,  /// output
	FALSE, /// can hav none
	FALSE, /// can have many
	&CLSID_NULL, // obs
	L"PIN",
	1,
	&PinTypes
};

static HRESULT registerfilter()
{
	IFilterMapper2* pFM = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void**)&pFM);
	if (SUCCEEDED(hr)) {
		////
		IMoniker *pMoniker = 0;
		REGFILTER2 rf2;
		rf2.dwVersion = 1;
		rf2.dwMerit = MERIT_DO_NOT_USE;
		rf2.cPins = 1;
		rf2.rgPins = &VCamPins;

		hr = pFM->RegisterFilter(CLSID_QWCamDShow, L"QW DShow VCamera", &pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &rf2);
		pFM->Release();
	}

	return hr;
}
static HRESULT dllregisterserver()
{
	//获取当前模块路径
	char path[MAX_PATH] = {0};
	char ch_key[MAX_PATH] = {0};
	char ch_clsid[MAX_PATH] = {0};
	LPOLESTR str;
	HRESULT hr = S_FALSE;

	GetModuleFileName(g_moduleinstall, path, MAX_PATH);
	
	::StringFromIID(CLSID_QWCamDShow, &str);
	WideCharToMultiByte(CP_ACP,0,str, -1, ch_clsid,sizeof(ch_clsid),nullptr,nullptr);
	::CoTaskMemFree(str);

	sprintf_s(ch_key, "%s%s", "CLSID\\", ch_clsid);

	CRegKey key;
	if (key.Create(HKEY_CLASSES_ROOT,ch_key) != 0)
	{
		return REGDB_E_CLASSNOTREG;
	}
	else
	{
		key.SetKeyValue("InprocServer32", path);
		key.SetKeyValue("InprocServer32", "Both", "ThreadingModel");
		hr = registerfilter();
	}
	return hr;
}

STDAPI DllUnregisterServer()
{
	return S_OK;
}
STDAPI DllRegisterServer()
{
	MessageBox(nullptr, "register", nullptr, 0);

	HRESULT hr = dllregisterserver();

	return hr;
}
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	MessageBox(nullptr, "DllGetClassObject", nullptr, 0);

	if (rclsid != CLSID_QWCamDShow)
	{
		return E_NOINTERFACE;
	}
	if (riid != IID_IUnknown && riid != IID_IClassFactory)
	{
		return E_NOINTERFACE;
	}

	CFactory *pFactoy = new CFactory(CLSID_QWCamDShow);
	
	HRESULT hr = pFactoy->QueryInterface(riid, ppv);
	if (hr != S_OK)
	{
		delete pFactoy;
		return E_NOINTERFACE;
	}
	return S_OK;
}
STDAPI DllCanUnloadNow(void)
{
	return S_OK;
}
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved)
{
	g_moduleinstall = hModule;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);
			printf("DLL_PROCESS_ATTACH\n");
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			printf("DLL_PROCESS_DETACH\n");
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			printf("DLL_THREAD_ATTACH\n");
			break;
		}
		case DLL_THREAD_DETACH:
		{
			printf("DLL_THREAD_DETACH.\n");
			break;
		}
		default:
			break;

	}
	return TRUE;
}