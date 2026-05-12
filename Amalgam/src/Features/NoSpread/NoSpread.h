#pragma once
#include "../../SDK/SDK.h"

class CNoSpread
{
private:
	bool ShouldRun(CTFWeaponBase* pWeapon);
	void StoreAngle(CUserCmd* pCmd, bool bFinal);

	Vec3 m_vAngles = {};
	Vec3 m_vOffset = {};

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	Vec3 GetOffset();
};

ADD_FEATURE(CNoSpread, NoSpread);