#pragma once
#include "../../../SDK/SDK.h"

class CNoSpreadProjectile
{
private:
	bool ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CNoSpreadProjectile, NoSpreadProjectile)