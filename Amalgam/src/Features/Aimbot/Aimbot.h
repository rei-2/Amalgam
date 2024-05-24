#pragma once
#include "../../SDK/SDK.h"

class CAimbot
{
private:
	bool ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

public:
	bool Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAimbot, Aimbot)