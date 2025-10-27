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
			if (I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > TF_ARROW_MAX_CHARGE_TIME)
			{
				vAngAdd.x += float(SDK::RandomInt()) / VALVE_RAND_MAX * 12.f - 6.f;
				vAngAdd.y += float(SDK::RandomInt()) / VALVE_RAND_MAX * 12.f - 6.f;
			}
			break;
		case TF_WEAPON_SYRINGEGUN_MEDIC:
			vAngAdd.x += SDK::RandomFloat(-1.5f, 1.5f);
			vAngAdd.y += SDK::RandomFloat(-1.5f, 1.5f);
			break;
		case TF_WEAPON_GRENADELAUNCHER:
		case TF_WEAPON_CANNON:
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		{
			float flSpeed;
			if (pWeapon->GetWeaponID() != TF_WEAPON_PIPEBOMBLAUNCHER)
				flSpeed = SDK::AttribHookValue(pLocal->InCond(TF_COND_RUNE_PRECISION) ? 3000.f : SDK::AttribHookValue(1200.f, "mult_projectile_speed", pWeapon), "mult_projectile_range", pWeapon);
			else
			{
				float flCharge = (pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() : 0.f);
				flSpeed = SDK::AttribHookValue(Math::RemapVal(flCharge, 0.f, SDK::AttribHookValue(4.f, "stickybomb_charge_rate", pWeapon), 900.f, 2400.f), "mult_projectile_range", pWeapon);
			}

			Vec3 vForward, vRight, vUp; Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, &vUp);
			Vec3 vVelocity = vForward * flSpeed + vUp * 200.f;
			Vec3 vNewVelocity = vVelocity + vUp * SDK::RandomFloat(-10.f, 10.f) + vRight * SDK::RandomFloat(-10.f, 10.f);

			vAngAdd = Math::VectorAngles(vNewVelocity) - Math::VectorAngles(vVelocity);
		}
		}

		if (!vAngAdd.IsZero())
		{
			pCmd->viewangles -= vAngAdd;
			G::PSilentAngles = true;
		}
	}
	}
}