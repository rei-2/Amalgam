#pragma once
#include "../../../SDK/SDK.h"

class CAutoRocketJump
{
	bool SetAngles(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	Vec3 m_vAngles = {};
	bool m_bFull = false;
	int m_iFrame = -1;
	int m_iDelay = 0;
	bool m_bRunning = false;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	bool IsRunning() { return m_bRunning; }
};

ADD_FEATURE(CAutoRocketJump, AutoRocketJump);