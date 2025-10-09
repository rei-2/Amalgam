#include "AutoAirblast.h"

#include "../AimbotProjectile/AimbotProjectile.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "../../Backtrack/Backtrack.h"

static inline float CalculateTimeToImpact(CTFPlayer* pLocal, CBaseEntity* pProjectile, const Vec3& vVelocity)
{
	Vec3 vLocalPos = pLocal->GetShootPos();
	Vec3 vProjPos = pProjectile->m_vecOrigin();
	
	float flDistance = vLocalPos.DistTo(vProjPos);
	float flSpeed = vVelocity.Length();
	
	if (flSpeed < 1.f)
		return 999.f;
	
	float flPing = F::Backtrack.GetReal();
	float flTimeToImpact = (flDistance / flSpeed) + flPing;
	
	static auto tf_flamethrower_flametime = U::ConVars.FindVar("tf_flamethrower_flametime");
	float flAirblastDelay = tf_flamethrower_flametime ? tf_flamethrower_flametime->GetFloat() : 0.1f;
	
	return std::max(flTimeToImpact - flAirblastDelay, 0.f);
}

static inline Vec3 PredictProjectilePosition(CBaseEntity* pProjectile, const Vec3& vVelocity, float flTime)
{
	Vec3 vOrigin = pProjectile->m_vecOrigin();
	Vec3 vPredicted = vOrigin + vVelocity * flTime;
	
	auto classID = pProjectile->GetClassID();
	if (classID == ETFClassID::CTFGrenadePipebombProjectile || 
		classID == ETFClassID::CTFProjectile_Arrow)
	{
		static auto sv_gravity = U::ConVars.FindVar("sv_gravity");
		float flGravity = sv_gravity ? sv_gravity->GetFloat() : 800.f;
		
		if (auto pWeapon = F::ProjSim.GetEntities(pProjectile).first)
			flGravity *= SDK::AttribHookValue(1, "mult_projectile_range", pWeapon);
		
		// s = v*t + 0.5*a*t^2
		vPredicted.z -= 0.5f * flGravity * flTime * flTime;
	}
	
	return vPredicted;
}

static inline bool ShouldTarget(CBaseEntity* pProjectile, CTFPlayer* pLocal)
{
	if (pProjectile->m_iTeamNum() == pLocal->m_iTeamNum())
		return false;

	switch (pProjectile->GetClassID())
	{
	case ETFClassID::CTFGrenadePipebombProjectile:
		if (pProjectile->As<CTFGrenadePipebombProjectile>()->m_bTouched())
			return false;
		break;
	}

	if (auto pWeapon = F::ProjSim.GetEntities(pProjectile).first)
	{
		if (!SDK::AttribHookValue(1, "mult_dmg", pWeapon))
			return false;
	}

	return true;
}

bool CAutoAirblast::CanAirblastEntity(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pEntity, Vec3& vAngle)
{
	auto flRadius = SDK::AttribHookValue(1, "deflection_size_multiplier", pWeapon) * 128.f;

	Vec3 vForward; Math::AngleVectors(vAngle, &vForward);
	Vec3 vOrigin = pLocal->GetShootPos() + vForward * flRadius;

	CBaseEntity* pTarget;
	for (CEntitySphereQuery sphere(vOrigin, flRadius);
		pTarget = sphere.GetCurrentEntity();
		sphere.NextEntity())
	{
		if (pTarget == pEntity)
			break;
	}

	return pTarget == pEntity && SDK::VisPosWorld(pLocal, pEntity, pLocal->GetShootPos(), pEntity->GetAbsOrigin());
}

void CAutoAirblast::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!(Vars::Aimbot::Projectile::AutoAirblast.Value & Vars::Aimbot::Projectile::AutoAirblastEnum::Enabled) || !G::CanSecondaryAttack)
		return;

	const int iWeaponID = pWeapon->GetWeaponID();
	if (iWeaponID != TF_WEAPON_FLAMETHROWER && iWeaponID != TF_WEAPON_FLAME_BALL || SDK::AttribHookValue(0, "airblast_disabled", pWeapon))
		return;

	static auto tf_flamethrower_burstammo = U::ConVars.FindVar("tf_flamethrower_burstammo");
	int iAmmoPerShot = tf_flamethrower_burstammo->GetInt() * SDK::AttribHookValue(1, "mult_airblast_cost", pWeapon);
	int iAmmo = pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType());
	int iBuffType = SDK::AttribHookValue(0, "set_buff_type", pWeapon);
	int iChargedAirblast = SDK::AttribHookValue(0, "set_charged_airblast", pWeapon);
	if (iAmmo < iAmmoPerShot || iBuffType || iChargedAirblast)
		return;

	bool bShouldBlast = false;
	const Vec3 vEyePos = pLocal->GetShootPos();

	float flLatency = std::max(F::Backtrack.GetReal() - 0.03f, 0.f);
	for (auto pProjectile : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
	{
		if (!ShouldTarget(pProjectile, pLocal))
			continue;

		Vec3 vOrigin;
		if (!SDK::PredictOrigin(vOrigin, pProjectile->m_vecOrigin(), F::ProjSim.GetVelocity(pProjectile), flLatency))
			continue;

		if (!(Vars::Aimbot::Projectile::AutoAirblast.Value & Vars::Aimbot::Projectile::AutoAirblastEnum::IgnoreFOV)
			&& Math::CalcFov(I::EngineClient->GetViewAngles(), Math::CalcAngle(vEyePos, vOrigin)) > Vars::Aimbot::General::AimFOV.Value)
			continue;

		Vec3 vRestoreOrigin = pProjectile->GetAbsOrigin();
		pProjectile->SetAbsOrigin(vOrigin);
		if (Vars::Aimbot::Projectile::AutoAirblast.Value & Vars::Aimbot::Projectile::AutoAirblastEnum::Redirect)
		{
			Vec3 vAngle = Math::CalcAngle(vEyePos, vOrigin);
			if (CanAirblastEntity(pLocal, pWeapon, pProjectile, vAngle))
			{
				bShouldBlast = true;
				if (!F::AimbotProjectile.AutoAirblast(pLocal, pWeapon, pCmd, pProjectile))
				{
					SDK::FixMovement(pCmd, vAngle);
					pCmd->viewangles = vAngle;
					G::PSilentAngles = true;
				}
			}
		}
		else if (CanAirblastEntity(pLocal, pWeapon, pProjectile, pCmd->viewangles))
			bShouldBlast = true;
		pProjectile->SetAbsOrigin(vRestoreOrigin);

		if (bShouldBlast)
			break;
	}

	if (bShouldBlast)
	{
		G::Attacking = true;
		pCmd->buttons |= IN_ATTACK2;
	}
}