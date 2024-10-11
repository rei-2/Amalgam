#pragma once
#include "../../../SDK/SDK.h"

class CFakeLag
{
	Vec3 vLastPosition;
	bool bPreservingBlast = false;
	bool bUnducking = false;

	bool IsAllowed(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void Prediction(CTFPlayer* pLocal, CUserCmd* pCmd);
	void PreserveBlastJump(CTFPlayer* pLocal);
	void Unduck(CTFPlayer* pLocal, CUserCmd* pCmd);

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool* pSendPacket);
};

ADD_FEATURE(CFakeLag, FakeLag)
