#pragma once
#include "../../SDK/SDK.h"
#include "FakeLag/FakeLag.h"
#include "AntiAim/AntiAim.h"

class CPacketManip
{
	bool WillTimeOut();
	bool AntiAimCheck(CTFPlayer* pLocal);

public:
	void Run(CTFPlayer* pLocal, CUserCmd* pCmd, bool* pSendPacket);
};

ADD_FEATURE(CPacketManip, PacketManip)