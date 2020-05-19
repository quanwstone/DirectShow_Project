#include "stdafx.h"
#include "CMediaFoundation.h"

static const GUID CLSID_QWCamDShow =
{ 0x6547a25b, 0xa535, 0x4d13,{ 0x97, 0x5d, 0xec, 0x13, 0xd9, 0x76, 0x7e, 0xb0 } };

CCMediaFoundation::CCMediaFoundation()
{
}


CCMediaFoundation::~CCMediaFoundation()
{
}

HRESULT CCMediaFoundation::InitMediaFoundation()
{
	HRESULT hr = S_OK;
	
	hr = CoInitialize(nullptr);
	if (!SUCCEEDED(hr))
	{
		return E_INVALIDARG;
	}
	
	//初始化
	hr = MFStartup(MF_VERSION);
	if (FAILED(hr))
	{
		TRACE("MFStartup failed\n");
	}

	IMFMediaSource *ppSource = nullptr;

	CreateVideoDeviceSource(&ppSource);
	return hr;
}

HRESULT CCMediaFoundation::CreateVideoDeviceSource(IMFMediaSource ** ppSource)
{
	*ppSource = nullptr;

	IMFMediaSource *pSource = nullptr;
	IMFAttributes *pAttributes = nullptr;
	IMFActivate **ppDevices = nullptr;

	HRESULT hr = MFCreateAttributes(&pAttributes, 1);
	if (FAILED(hr))
	{
		TRACE("MFCreateAttributes Failed\n");
	}

	hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
		MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
	if (FAILED(hr))
	{
		TRACE("SetGUID Failed\n");
	}

	UINT32 count;
	hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
	if (FAILED(hr) || count == 0)
	{
		TRACE("MFEnumDeviceSources Failed\n");
	}

	hr = ppDevices[0]->ActivateObject(IID_PPV_ARGS(&pSource));
	if (FAILED(hr))
	{
		TRACE("ActivateObject Failed\n");
	}
	
	*ppSource = pSource;
	(*ppSource)->AddRef();

	//EnumDeviceMediaType(pSource);

	//创建Source Reader
	IMFSourceReader *pSourceReader = nullptr;

	hr = MFCreateSourceReaderFromMediaSource(*ppSource, nullptr/*pAttributes*/, &pSourceReader);
	if (FAILED(hr))
	{
		TRACE("MFCreateSourceReaderFromMediaSource Failed\n");
	}
	//设置Media Type
	IMFMediaType *mediatype = nullptr;
	
	hr = MFCreateMediaType(&mediatype);
	mediatype->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	mediatype->SetGUID(MF_MT_SUBTYPE, WMMEDIASUBTYPE_YUY2);	//设置支持的类型 否则读取失败
	hr = MFSetAttributeSize(mediatype, MF_MT_FRAME_SIZE, 640, 480);

	hr = pSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, mediatype);
	//

	IMFSample *sampe = nullptr;
	IMFMediaBuffer *pbuffer = nullptr;
	
	DWORD index = 0, flags = 0;
	LONGLONG llvideoTs = 0;
	BYTE *pbyte = nullptr;
	DWORD iMax = 640 * 480 * 2;
	DWORD iCurrent = 0;
	pbyte = new BYTE[iMax];
	
	while (true)
	{
		hr = pSourceReader->ReadSample(MF_SOURCE_READER_ANY_STREAM,
			0, &index, &flags, &llvideoTs, &sampe);
		if (FAILED(hr))
		{
			TRACE("");
			break;
		}
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			TRACE("end\n");
			break;
		}

		if (sampe)
		{
			BYTE *data = nullptr;
			IMFMediaBuffer *buffer = nullptr;
			DWORD dmax=0, dcurrent = 0;
			
			sampe->GetBufferByIndex(0, &buffer);
			buffer->Lock(&data, &dmax, &dcurrent);
			buffer->Unlock();
			buffer->Release();
		}
	}

	pAttributes->Release();
	for (int i = 0; i < count; i++)
	{
		ppDevices[i]->Release();
	}
	CoTaskMemFree(ppDevices);
	pSource->Release();

	return hr;
}

HRESULT CCMediaFoundation::EnumDeviceMediaType(IMFMediaSource * pSource)
{
	HRESULT hr = S_OK;
	IMFPresentationDescriptor *pDescriptor;
	IMFStreamDescriptor	*pStreamDes;
	IMFMediaTypeHandler	*pMediaHand;
	IMFMediaType	*pMediaType;
	BOOL bSelect;
	DWORD dcount;
	UINT32 u32;
	GUID guid;

	hr = pSource->CreatePresentationDescriptor(&pDescriptor);
	hr = pDescriptor->GetStreamDescriptorByIndex(0, &bSelect, &pStreamDes);
	hr = pStreamDes->GetMediaTypeHandler(&pMediaHand);
	pMediaHand->GetMediaTypeCount(&dcount);

	for (int i = 0; i < dcount; i++)
	{
		pMediaHand->GetMediaTypeByIndex(i, &pMediaType);

		pMediaType->GetCount(&u32);
		for (int j = 0; j < u32; j++)
		{
			PROPVARIANT var;
			PropVariantInit(&var);

			pMediaType->GetItemByIndex(j, &guid, &var);
			switch (var.vt)
			{
			case VT_UI8:
				TRACE("var:%d\n", var.uhVal);
				break;
			case VT_LPSTR:
				TRACE("VAR:%s\n", var.bstrVal);
				break;
			defalut:
				break;
			}
			PropVariantClear(&var);
		}

		pMediaType->Release();
	}

	return E_NOTIMPL;
}
