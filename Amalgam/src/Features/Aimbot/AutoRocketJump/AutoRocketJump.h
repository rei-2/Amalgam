#pragma once
#include "../../../SDK/SDK.h"

class CAutoRocketJump
{
	void ManageAngle(CTFWeaponBase* pWeapon, CUserCmd* pCmd, Vec3& viewAngles);

	bool bFull = false;
	int iFrame = -1;
	int iDelay = 0;

	bool bRunning = false;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	bool IsRunning() { return bRunning; }
};

ADD_FEATURE(CAutoRocketJump, AutoRocketJump)