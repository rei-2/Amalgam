#pragma once
#include "../../../SDK/SDK.h"

class CAutoRocketJump
{
	void ManageAngle(CTFWeaponBase* pWeapon, CUserCmd* pCmd, Vec3& viewAngles);

	bool bLastGrounded = false;

	bool bFull = false;
	int iDelay = 0;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	int iFrame = -1;
};

ADD_FEATURE(CAutoRocketJump, AutoRocketJump)