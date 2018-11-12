/*
BSTR:定义为OLEWCHAR 或OLECHAR，

注意：
引用Strmiids.lib：无法解析的外部符号_IID_ICreateDevEnum

查找输入设备流程如下：
1:获取系统设备组件对象ICreateDevEnum CLSID_SystemDeviceEnum.
2:通过组件对象获取指定类型的设备枚举对象CreateClassEnumerator CLSID_VideoInputDeviceCategory
3:通过获取的指定类型的枚举对象，遍历设备，获取设备枚举对象pMoniker
4:通过设备枚举对象获取设备详细信息BindToStorage
5:释放com引用 
*/
#include<stdio.h>
#include<dshow.h>
#pragma comment(lib, "Strmiids.lib")

char *WCHARConvToCHAR(WCHAR *pSrc)
{
	int nSize = wcslen(pSrc);

	int nDst = WideCharToMultiByte(CP_ACP, 0, pSrc, nSize, NULL, 0, 0, 0);
	
	char *pDst = new char[nDst + 1];
	memset(pDst,0,nDst + 1);

	WideCharToMultiByte(CP_ACP, 0, pSrc, nSize, pDst, nDst + 1, 0, 0);

	return pDst;
}
HRESULT GetDeviceEnum()
{
	HRESULT hr = S_OK;

	ICreateDevEnum *lpDevEnum = NULL;
	
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&lpDevEnum);
	if (!SUCCEEDED(hr))
	{
		printf("CoCreateInstance Failed\n");
		return hr;
	}
	//
	IEnumMoniker *pEnum = NULL;
	
	hr = lpDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
	if (!SUCCEEDED(hr))
	{
		printf("CreateClassEnumerator Failed\n");
		return hr;
	}
	//
	IMoniker *pMoniker = NULL;
	ULONG ulFected = 0;

	while (pEnum->Next(1, &pMoniker, &ulFected) == S_OK)
	{
		IPropertyBag *pProBag = NULL;
		
		hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void**)&pProBag);
		if (SUCCEEDED(hr))
		{
			VARIANT varName;
			VariantInit(&varName);
			
			hr = pProBag->Read(L"FriendlyName", &varName, 0);
			if (SUCCEEDED(hr))
			{
				char *lptstrValue = WCHARConvToCHAR(varName.bstrVal);

				printf("Device Name = %s\n", lptstrValue);

				delete[] lptstrValue;
			}
			VariantClear(&varName);	
			pProBag->Release();
		}
		pMoniker->Release();
	}
	pEnum->Release();
	lpDevEnum->Release();

	return hr;
}
int main(int argc, char *argv[])
{
	HRESULT hr = CoInitialize(NULL);
	
	if (!SUCCEEDED(hr))
	{
		printf("Coinitialize Failed\n");
	}

	hr = GetDeviceEnum();
	if (FAILED(hr))
	{
		printf("GetDeviceEnum Failed\n");
	}

	CoUninitialize();
	
	for (;;);

	return 0;
}