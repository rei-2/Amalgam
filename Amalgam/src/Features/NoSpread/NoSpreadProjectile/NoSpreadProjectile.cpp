#include "NoSpreadProjectile.h"

bool CNoSpreadProjectile::ShouldRun(CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (G::PrimaryWeaponType != EWeaponType::PROJECTILE)
		return false;

	return G::Attacking == 1;
}

void CNoSpreadProjectile::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!ShouldRun(pLocal, pWeapon))
		return;

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_RAYGUN:
	case TF_WEAPON_DRG_POMSON:
	case TF_WEAPON_GRENADELAUNCHER:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_CANNON:
	case TF_WEAPON_FLAREGUN:
	case TF_WEAPON_FLAREGUN_REVENGE:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	case TF_WEAPON_GRAPPLINGHOOK:
	{
		SDK::RandomSeed(SDK::SeedFileLineHash(MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF, "SelectWeightedSequence", 0));
		for (int i = 0; i < 6; ++i)
			SDK::RandomFloat();

		Vec3 vAngAdd = pWeapon->GetSpreadAngles() - pLocal->EyeAngles();
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_COMPOUND_BOW:
			if (pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f && I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 5.0f)
			{
				vAngAdd.x += -6.f + float(SDK::RandomInt()) / 0x7FFF * 12.f;
				vAngAdd.y += -6.f + float(SDK::RandomInt()) / 0x7FFF * 12.f;
			}
			break;
		case TF_WEAPON_SYRINGEGUN_MEDIC:
			vAngAdd.x += SDK::RandomFloat(-1.5f, 1.5f);
			vAngAdd.y += SDK::RandomFloat(-1.5f, 1.5f);
		}

		if (!vAngAdd.IsZero())
		{
			pCmd->viewangles -= vAngAdd;
			G::PSilentAngles = true;
		}
	}
	}
}