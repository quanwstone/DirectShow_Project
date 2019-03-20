#pragma once

#include"AudioCapture.h"
#include"VideoCapture.h"

class CCaptureManager
{
public:
	CCaptureManager();
	~CCaptureManager();

private:
	CAudioCapture	*m_pAudioCapture;
	CVideoCapture	*m_pVideoCapture;

public:
	/*
	说明：设备初始化
	参数：in_hwnd窗口句柄
	返回值：成功返回TRUE，失败返回FALSE.
	*/
	BOOL	InitDevice(HWND in_hwnd);
	
	/*
	说明：打开设备.
	参数：in_chVName视频设备名称,in_chAName音频设备名称
	返回值：成功返回TRUE，失败返回FALSE.
	*/	
	BOOL	OpenDevice(char *in_chVName,char *in_chAName);

	/*
	说明：获取设备列表
	参数：out_chVList视频设备列表，out_chAList音频设备列表
	返回值：成功返回TRUE，失败返回FALSE.
	*/
	BOOL	GetDeviceList(char *out_chVList[], char *out_chAList[]);

	/*
	说明：开始采集
	返回值：成功返回TRUE，失败返回FALSE.
	*/
	BOOL	StartCapture();

	/*
	说明：停止采集
	返回值：成功返回TRUE，失败返回FALSE.
	*/
	BOOL	StopCapture();

	/*
	说明：关闭打开的设备
	返回值：成功返回TRUE,失败返回FALSE.
	*/
	BOOL	CloseDevice();
};

