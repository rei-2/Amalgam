#include "NoSpread.h"

#include "NoSpreadProjectile/NoSpreadProjectile.h"
#include "NoSpreadHitscan/NoSpreadHitscan.h"

bool CNoSpread::ShouldRun(CTFWeaponBase* pWeapon)
{
	if (!Vars::Aimbot::General::NoSpread.Value
		|| !pWeapon || G::Attacking != 1)
		return false;

	return true;
}

void CNoSpread::StoreAngle(CUserCmd* pCmd, bool bFinal)
{
	if (!Vars::Aimbot::General::NoSpread.Value)
	{
		m_vOffset = {};
		return;
	}

	if (G::PrimaryWeaponType == EWeaponType::HITSCAN && I::ClientState->chokedcommands && m_vOffset)
		return;

	if (!bFinal)
		m_vAngles = pCmd->viewangles;
	else
		m_vOffset = pCmd->viewangles - m_vAngles;
}

void CNoSpread::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	StoreAngle(pCmd, false);
	if (!ShouldRun(pWeapon))
		return;

	F::NoSpreadHitscan.Run(pLocal, pWeapon, pCmd);
	F::NoSpreadProjectile.Run(pLocal, pWeapon, pCmd);
	StoreAngle(pCmd, true);
}

Vec3 CNoSpread::GetOffset()
{
	return m_vOffset;
}