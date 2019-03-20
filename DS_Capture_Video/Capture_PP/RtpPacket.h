#pragma once
class CRtpPacket
{
public:
	CRtpPacket();
	~CRtpPacket();

public:
	//
	bool CreateRtpInstance();

	//
	bool DestoryRtpInstance();

	//4Byte
	bool SetTimeStamp(DWORD dwTime);

	//4Byte
	bool SetSSRCId(DWORD dwID);

	//2Byte
	bool SetSequNumber(short shNum);

};

