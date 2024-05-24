#include "Aimbot.h"

#include "AimbotHitscan/AimbotHitscan.h"
#include "AimbotProjectile/AimbotProjectile.h"
#include "AimbotMelee/AimbotMelee.h"
#include "AutoDetonate/AutoDetonate.h"
#include "AutoAirblast/AutoAirblast.h"
#include "AutoUber/AutoUber.h"
#include "AutoRocketJump/AutoRocketJump.h"
#include "../Misc/Misc.h"

bool CAimbot::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (!pLocal || !pWeapon
		|| !pLocal->IsAlive()
		|| pLocal->IsTaunting()
		|| pLocal->IsBonked()
		|| pLocal->m_bFeignDeathReady()
		|| pLocal->IsCloaked()
		|| pLocal->IsInBumperKart()
		|| pLocal->IsAGhost())
		return false;

	switch (G::WeaponDefIndex)
	{
	case Soldier_m_RocketJumper:
	case Demoman_s_StickyJumper:
		return false;
	}

	if (I::EngineVGui->IsGameUIVisible() || I::MatSystemSurface->IsCursorVisible())
		return false;

	return true;
}

bool CAimbot::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	G::AimPosition = Vec3();
	if (pCmd->weaponselect)
		return false;

	F::AutoRocketJump.Run(pLocal, pWeapon, pCmd);

	if (!ShouldRun(pLocal, pWeapon))
		return false;

	F::AutoDetonate.Run(pLocal, pWeapon, pCmd);
	F::AutoAirblast.Run(pLocal, pWeapon, pCmd);
	F::AutoUber.Run(pLocal, pWeapon, pCmd);

	const bool bAttacking = G::IsAttacking;
	switch (G::WeaponType)
	{
	case EWeaponType::HITSCAN: F::AimbotHitscan.Run(pLocal, pWeapon, pCmd); break;
	case EWeaponType::PROJECTILE: F::AimbotProjectile.Run(pLocal, pWeapon, pCmd); break;
	case EWeaponType::MELEE: F::AimbotMelee.Run(pLocal, pWeapon, pCmd); break;
	}

	switch (pWeapon->m_iWeaponID())
	{
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_STICKY_BALL_LAUNCHER:
	case TF_WEAPON_GRENADE_STICKY_BALL:
	case TF_WEAPON_CANNON:
		if (!(G::Buttons & IN_ATTACK) && pCmd->buttons & IN_ATTACK)
			return true;
	}
	return bAttacking != G::IsAttacking;
}
