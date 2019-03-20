#include "stdafx.h"
#include "Utill.h"
#include "CaptureManager.h"


CCaptureManager::CCaptureManager()
{
	m_pVideoCapture = nullptr;
	m_pAudioCapture = nullptr;

}


CCaptureManager::~CCaptureManager()
{
}

//初始化设备信息
BOOL CCaptureManager::InitDevice(HWND in_hwnd)
{
	m_pVideoCapture = new CVideoCapture();	

	if (m_pVideoCapture)
	{
		BOOL bre = m_pVideoCapture->InitVideoDevice(in_hwnd);

		if (!bre)
		{
			TRACE("CCaptureManager::InitDevice Failed.");
			DEF_SAFE_DELETE_OBJECT(m_pVideoCapture);
			return FALSE;
		}
	}

	m_pAudioCapture = new CAudioCapture();

	if (m_pAudioCapture)
	{
		
	}

	return  TRUE;
}
//打开设备
BOOL CCaptureManager::OpenDevice(char *in_chVName, char *in_chAName)
{	
	if (!m_pVideoCapture)
		return FALSE;

	BOOL bre = m_pVideoCapture->OpenDevice(in_chVName);	
	if (!bre)
	{
		return FALSE;
	}

	return  TRUE;
}
//获取设备列表
BOOL CCaptureManager::GetDeviceList(char *out_chVList[], char *out_chAList[])
{
	BOOL bre = FALSE;

	if (!m_pVideoCapture)
		return FALSE;

	bre = m_pVideoCapture->GetDeviceList(out_chVList);

	return  bre;
}
//开始采集
BOOL CCaptureManager::StartCapture()
{
	BOOL bre = FALSE;
	
	if(m_pVideoCapture)
		bre = m_pVideoCapture->Start();

	return  bre;
}
//停止采集
BOOL CCaptureManager::StopCapture()
{
	BOOL bre = FALSE;

	return  bre;
}
//关闭设备
BOOL CCaptureManager::CloseDevice()
{
	BOOL bre = FALSE;

	if (m_pVideoCapture)
		m_pVideoCapture->CloseDevice();

	return  bre;
}