#pragma once
#include "../../../SDK/SDK.h"

#include "../AimbotGlobal/AimbotGlobal.h"

class CAutoDetonate
{
	bool CheckDetonation(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, EGroupType entityGroup, float flRadiusScale, CUserCmd* pCmd);

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAutoDetonate, AutoDetonate)