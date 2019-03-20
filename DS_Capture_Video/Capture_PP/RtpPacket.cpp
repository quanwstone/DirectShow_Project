#include "stdafx.h"
#include "RtpPacket.h"


CRtpPacket::CRtpPacket()
{
}


CRtpPacket::~CRtpPacket()
{
}
//
bool CRtpPacket::CreateRtpInstance()
{
	return true;
}
//
bool CRtpPacket::DestoryRtpInstance()
{
	return true;
}

bool CRtpPacket::SetTimeStamp(DWORD dwTime)
{
	return true;
}

bool CRtpPacket::SetSSRCId(DWORD dwID)
{
	return true;
}

bool CRtpPacket::SetSequNumber(short shNum)
{
	return true;
}