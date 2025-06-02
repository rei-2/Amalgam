#pragma once
#include "../../../SDK/SDK.h"

class CFakeLag
{
	bool IsAllowed(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void Prediction(CTFPlayer* pLocal, CUserCmd* pCmd);
	void PreserveBlastJump(CTFPlayer* pLocal);
	void Unduck(CTFPlayer* pLocal, CUserCmd* pCmd);

	Vec3 m_vLastPosition = {};
	bool m_bPreservingBlast = false;
	bool m_bUnducking = false;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool* pSendPacket);

	int m_iGoal = 0;
};

ADD_FEATURE(CFakeLag, FakeLag);