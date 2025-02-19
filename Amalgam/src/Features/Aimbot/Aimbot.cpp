#include "Aimbot.h"

#include "AimbotHitscan/AimbotHitscan.h"
#include "AimbotProjectile/AimbotProjectile.h"
#include "AimbotMelee/AimbotMelee.h"
#include "AutoDetonate/AutoDetonate.h"
#include "AutoAirblast/AutoAirblast.h"
#include "AutoHeal/AutoHeal.h"
#include "AutoRocketJump/AutoRocketJump.h"
#include "../Misc/Misc.h"
#include "../Visuals/Visuals.h"

bool CAimbot::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (!pLocal || !pWeapon
		|| !pLocal->IsAlive()
		|| pLocal->IsAGhost()
		|| pLocal->IsTaunting()
		|| pLocal->InCond(TF_COND_STUNNED) && pLocal->m_iStunFlags() & (TF_STUN_CONTROLS | TF_STUN_LOSER_STATE)
		|| pLocal->m_bFeignDeathReady()
		|| pLocal->InCond(TF_COND_PHASE)
		|| pLocal->InCond(TF_COND_STEALTHED)
		|| pLocal->InCond(TF_COND_HALLOWEEN_KART))
		return false;

	switch (pWeapon->m_iItemDefinitionIndex())
	{
	case Soldier_m_RocketJumper:
	case Demoman_s_StickyJumper:
		return false;
	}

	if (I::EngineVGui->IsGameUIVisible() || I::MatSystemSurface->IsCursorVisible())
		return false;

	return true;
}

void CAimbot::RunAimbot(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, bool bSecondaryType)
{
	EWeaponType eWeaponType = !bSecondaryType ? G::PrimaryWeaponType : G::SecondaryWeaponType;

	bool bOriginal;
	if (bSecondaryType)
		bOriginal = G::CanPrimaryAttack, G::CanPrimaryAttack = G::CanSecondaryAttack;

	switch (eWeaponType)
	{
	case EWeaponType::HITSCAN: F::AimbotHitscan.Run(pLocal, pWeapon, pCmd); break;
	case EWeaponType::PROJECTILE: F::AimbotProjectile.Run(pLocal, pWeapon, pCmd); break;
	case EWeaponType::MELEE: F::AimbotMelee.Run(pLocal, pWeapon, pCmd); break;
	}

	if (bSecondaryType)
		G::CanPrimaryAttack = bOriginal;
}

void CAimbot::RunMain(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (F::AimbotProjectile.m_bLastTickCancel)
	{
		pCmd->weaponselect = F::AimbotProjectile.m_bLastTickCancel;
		F::AimbotProjectile.m_bLastTickCancel = 0;
	}

	bRan = false;
	if (abs(G::Target.second - I::GlobalVars->tickcount) > 32)
		G::Target = { 0, 0 };
	if (abs(G::AimPosition.second - I::GlobalVars->tickcount) > 32)
		G::AimPosition = { {}, 0 };

	if (pCmd->weaponselect)
		return;

	F::AutoRocketJump.Run(pLocal, pWeapon, pCmd);
	if (!ShouldRun(pLocal, pWeapon))
		return;

	F::AutoDetonate.Run(pLocal, pWeapon, pCmd);
	F::AutoAirblast.Run(pLocal, pWeapon, pCmd);
	F::AutoHeal.Run(pLocal, pWeapon, pCmd);

	RunAimbot(pLocal, pWeapon, pCmd);
	RunAimbot(pLocal, pWeapon, pCmd, true);
}

void CAimbot::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	RunMain(pLocal, pWeapon, pCmd);

	G::Attacking = SDK::IsAttacking(pLocal, pWeapon, pCmd, true);
}