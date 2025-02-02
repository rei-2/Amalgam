#include "NoSpread.h"

#include "NoSpreadProjectile/NoSpreadProjectile.h"
#include "NoSpreadHitscan/NoSpreadHitscan.h"

bool CNoSpread::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (!Vars::Aimbot::General::NoSpread.Value)
		return false;

	if (!pLocal || !pWeapon
		|| !pLocal->IsAlive()
		|| pLocal->IsAGhost()
		|| pLocal->IsTaunting()
		|| pLocal->InCond(TF_COND_STUNNED) && pLocal->m_iStunFlags() & (TF_STUN_CONTROLS | TF_STUN_LOSER_STATE)
		|| pLocal->m_bFeignDeathReady()
		|| pLocal->InCond(TF_COND_PHASE)
		|| pLocal->InCond(TF_COND_STEALTHED)
		|| pLocal->InCond(TF_COND_HALLOWEEN_KART))
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