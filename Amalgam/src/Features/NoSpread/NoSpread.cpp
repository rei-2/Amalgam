#include "NoSpread.h"

#include "NoSpreadProjectile/NoSpreadProjectile.h"
#include "NoSpreadHitscan/NoSpreadHitscan.h"

bool CNoSpread::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (!Vars::Aimbot::General::NoSpread.Value)
		return false;

	if (!pLocal || !pWeapon
		|| !pLocal->IsAlive()
		|| pLocal->IsTaunting()
		|| pLocal->IsBonked()
		|| pLocal->m_bFeignDeathReady()
		|| pLocal->IsCloaked()
		|| pLocal->IsInBumperKart()
		|| pLocal->IsAGhost())
	{
		return false;
	}

	return true;
}

void CNoSpread::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!ShouldRun(pLocal, pWeapon))
		return;

	F::NoSpreadHitscan.Run(pLocal, pWeapon, pCmd);
	F::NoSpreadProjectile.Run(pLocal, pWeapon, pCmd);
}