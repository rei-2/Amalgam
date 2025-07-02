#pragma once
#include "../../../SDK/SDK.h"

class CAutoAirblast
{
public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	bool CanAirblastEntity(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pEntity, Vec3& vAngle);
};

ADD_FEATURE(CAutoAirblast, AutoAirblast);