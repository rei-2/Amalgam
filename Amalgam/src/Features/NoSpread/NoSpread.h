#pragma once
#include "../../SDK/SDK.h"

class CNoSpread
{
private:
	bool ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CNoSpread, NoSpread);