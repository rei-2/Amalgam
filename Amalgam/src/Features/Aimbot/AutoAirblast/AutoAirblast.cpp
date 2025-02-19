#include "AutoAirblast.h"

#include "../../Backtrack/Backtrack.h"

static inline bool ShouldTargetProjectile(CBaseEntity* pProjectile, CTFPlayer* pLocal)
{
	if (pProjectile->m_iTeamNum() == pLocal->m_iTeamNum())
		return false;

	switch (pProjectile->GetClassID())
	{
	case ETFClassID::CTFGrenadePipebombProjectile:
	case ETFClassID::CTFProjectile_Cleaver:
	case ETFClassID::CTFStunBall:
	{
		if (pProjectile->As<CTFGrenadePipebombProjectile>()->m_bTouched())
			return false; // ignore landed vphysics objects
		break;
	}
	case ETFClassID::CTFProjectile_Arrow:
	{
		if (pProjectile->GetAbsVelocity().IsZero())
			return false; // ignore arrows with no velocity / not moving
	}
	}

	return true;
}

static inline Vec3 PredictOrigin(Vec3& vOrigin, Vec3 vVelocity, float flLatency, bool bTrace = true, Vec3 vMins = {}, Vec3 vMaxs = {}, unsigned int nMask = MASK_SOLID)
{
	if (vVelocity.IsZero() || !flLatency)
		return vOrigin;

	Vec3 vTo = vOrigin + vVelocity * flLatency;
	if (!bTrace)
		return vTo;

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};

	SDK::TraceHull(vOrigin, vTo, vMins, vMaxs, nMask, &filter, &trace);
	return vOrigin + (vTo - vOrigin) * trace.fraction;
}

bool CAutoAirblast::CanAirblastEntity(CTFPlayer* pLocal, CBaseEntity* pEntity, Vec3& vAngle, Vec3& vPos)
{
	Vec3 vForward; Math::AngleVectors(vAngle, &vForward);
	const Vec3 vOrigin = pLocal->GetShootPos() + (vForward * 128.f);

	CBaseEntity* pTarget;
	for (CEntitySphereQuery sphere(vOrigin, 128.f);
		(pTarget = sphere.GetCurrentEntity()) != nullptr;
		sphere.NextEntity())
	{
		if (pTarget == pEntity)
			break;
	}

	return pTarget == pEntity && SDK::VisPos(pLocal, pEntity, pLocal->GetShootPos(), vPos);
}

void CAutoAirblast::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!(Vars::Aimbot::Projectile::AutoAirblast.Value & Vars::Aimbot::Projectile::AutoAirblastEnum::Enabled) || !G::CanSecondaryAttack /*|| Vars::Auto::Airblast::DisableOnAttack.Value && pCmd->buttons & IN_ATTACK*/)
		return;

	const int iWeaponID = pWeapon->GetWeaponID();
	if (iWeaponID != TF_WEAPON_FLAMETHROWER && iWeaponID != TF_WEAPON_FLAME_BALL || pWeapon->m_iItemDefinitionIndex() == Pyro_m_ThePhlogistinator)
		return;

	const Vec3 vEyePos = pLocal->GetShootPos();
	bool bShouldBlast = false;

	float flLatency = std::max(F::Backtrack.GetReal() - 0.05f, 0.f);

	for (auto pProjectile : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
	{
		if (!ShouldTargetProjectile(pProjectile, pLocal))
			continue;

		Vec3 vRestoreOrigin = pProjectile->GetAbsOrigin();
		Vec3 vOrigin = PredictOrigin(pProjectile->m_vecOrigin(), pProjectile->GetAbsVelocity(), flLatency);
		pProjectile->SetAbsOrigin(vOrigin);

		if (Vars::Aimbot::Projectile::AutoAirblast.Value & Vars::Aimbot::Projectile::AutoAirblastEnum::RespectFOV
			&& Math::GetFov(I::EngineClient->GetViewAngles(), vEyePos, vOrigin) > Vars::Aimbot::General::AimFOV.Value)
			continue;

		/*
		if (Vars::Aimbot::Projectile::AutoAirblast.Value & Vars::Aimbot::Projectile::AutoAirblastEnum::RedirectAdvanced)
		{

		}
		else*/ if (Vars::Aimbot::Projectile::AutoAirblast.Value & Vars::Aimbot::Projectile::AutoAirblastEnum::RedirectSimple
			|| Vars::Aimbot::Projectile::AutoAirblast.Value & Vars::Aimbot::Projectile::AutoAirblastEnum::RedirectAdvanced)
		{
			Vec3 vAngle = Math::CalcAngle(vEyePos, vOrigin);
			if (CanAirblastEntity(pLocal, pProjectile, vAngle, vOrigin))
			{
				SDK::FixMovement(pCmd, vAngle);
				pCmd->viewangles = vAngle;
				G::PSilentAngles = true;
				bShouldBlast = true;
			}
		}
		else if (CanAirblastEntity(pLocal, pProjectile, pCmd->viewangles, vOrigin))
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