#pragma once
#include "../../../SDK/SDK.h"

class CAutoAirblast
{
	bool CanAirblastEntity(CTFPlayer* pLocal, CBaseEntity* pEntity, Vec3& vAngle, Vec3& vPos);

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAutoAirblast, AutoAirblast)