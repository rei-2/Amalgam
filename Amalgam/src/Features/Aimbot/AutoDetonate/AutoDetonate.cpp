#include "AutoDetonate.h"

void CAutoDetonate::PredictPlayers(CTFPlayer* pLocal, float flLatency, bool bLocal)
{
	if (!m_mRestore.empty())
		RestorePlayers();

	for (auto pEntity : H::Entities.GetGroup(EntityEnum::PlayerAll))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!bLocal
			? (pPlayer == pLocal || !pPlayer->IsAlive() || pPlayer->IsAGhost())
			: (pLocal != pPlayer || !pLocal->IsAlive() || pLocal->IsAGhost()))
			continue;

		m_mRestore[pPlayer] = pPlayer->GetAbsOrigin();

		pPlayer->SetAbsOrigin(SDK::PredictOrigin(pPlayer->m_vecOrigin(), pPlayer->m_vecVelocity(), flLatency, true, pPlayer->m_vecMins() + 0.125f, pPlayer->m_vecMaxs() - 0.125f, pPlayer->SolidMask()));
	}
}

void CAutoDetonate::RestorePlayers()
{
	for (auto& [pEntity, vRestore] : m_mRestore)
		pEntity->SetAbsOrigin(vRestore);
	m_mRestore.clear();
}

static inline bool GetRadius(EntityEnum::EntityEnum iGroup, CBaseEntity* pProjectile, float& flRadius, CTFWeaponBase*& pWeapon)
{
	if (iGroup == EntityEnum::LocalStickies)
		pWeapon = pProjectile->As<CTFGrenadePipebombProjectile>()->m_hOriginalLauncher()->As<CTFWeaponBase>();
	else
		pWeapon = pProjectile->As<CTFProjectile_Flare>()->m_hLauncher()->As<CTFWeaponBase>();
	if (!pWeapon)
	{
		pWeapon = H::Entities.GetWeapon();
		if (!pWeapon)
			return false;
	}

	if (iGroup == EntityEnum::LocalStickies)
	{
		auto pPipebomb = pProjectile->As<CTFGrenadePipebombProjectile>();
		if (!pPipebomb->m_flCreationTime() || I::GlobalVars->curtime < pPipebomb->m_flCreationTime() + SDK::AttribHookValue(0.8f, "sticky_arm_time", pWeapon))
			return false;

		flRadius *= 146.f;
		if (!pPipebomb->m_bTouched())
		{
			static auto tf_grenadelauncher_livetime = U::ConVars.FindVar("tf_grenadelauncher_livetime");
			static auto tf_sticky_radius_ramp_time = U::ConVars.FindVar("tf_sticky_radius_ramp_time");
			static auto tf_sticky_airdet_radius = U::ConVars.FindVar("tf_sticky_airdet_radius");
			float flLiveTime = tf_grenadelauncher_livetime->GetFloat();
			float flRampTime = tf_sticky_radius_ramp_time->GetFloat();
			float flAirdetRadius = tf_sticky_airdet_radius->GetFloat();
			flRadius *= Math::RemapVal(I::GlobalVars->curtime - pPipebomb->m_flCreationTime(), flLiveTime, flLiveTime + flRampTime, flAirdetRadius, 1.f);
		}
	}
	else
		flRadius *= 110.f;
	flRadius = SDK::AttribHookValue(flRadius, "mult_explosion_radius", pWeapon);
	return true;
}

static inline bool CheckEntities(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile, float flRadius, Vec3 vOrigin)
{
	flRadius -= 1;

	CBaseEntity* pEntity;
	for (CEntitySphereQuery sphere(vOrigin, flRadius);
		pEntity = sphere.GetCurrentEntity();
		sphere.NextEntity())
	{
		if (pEntity == pLocal || pEntity->IsPlayer() && (!pEntity->As<CTFPlayer>()->IsAlive() || pEntity->As<CTFPlayer>()->IsAGhost())
			|| !F::AimbotGlobal.FriendlyFire() && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
			continue;

		// CEntitySphereQuery actually does a box test so we need to make sure the distance is less than the radius first
		Vec3 vPos; reinterpret_cast<CCollisionProperty*>(pEntity->GetCollideable())->CalcNearestPoint(vOrigin, &vPos);
		if (vOrigin.DistTo(vPos) > flRadius)
			continue;

		if (F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
			continue;

		if (Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::IgnoreInvisible && pEntity->IsPlayer() && pEntity->As<CTFPlayer>()->IsInvisible(Vars::Aimbot::General::IgnoreInvisible.Value / 100.f))
			continue;

		if (!SDK::VisPosCollideable(pProjectile, pEntity, vOrigin, pEntity->IsPlayer() ? pEntity->GetAbsOrigin() + pEntity->As<CTFPlayer>()->GetViewOffset() : pEntity->GetCenter(), MASK_SHOT))
			continue;

		if (pCmd && pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER && pWeapon->As<CTFPipebombLauncher>()->GetDetonateType() == TF_DETONATE_MODE_DOT)
		{
			Vec3 vAngleTo = Math::CalcAngle(pLocal->GetShootPos(), vOrigin);
			SDK::FixMovement(pCmd, vAngleTo);
			pCmd->viewangles = vAngleTo;
			G::PSilentAngles = true;
		}

		return true;
	}

	return false;
}

bool CAutoDetonate::CheckDetonation(CTFPlayer* pLocal, EntityEnum::EntityEnum iGroup, float flRadiusScale, CUserCmd* pCmd)
{
	auto& vProjectiles = H::Entities.GetGroup(iGroup);
	if (vProjectiles.empty())
		return false;

	float flLatency = F::Backtrack.GetReal();

	for (auto pProjectile : vProjectiles)
	{
		float flRadius = flRadiusScale;
		CTFWeaponBase* pWeapon = nullptr;
		if (!GetRadius(iGroup, pProjectile, flRadius, pWeapon))
			continue;

		Vec3 vOrigin = SDK::PredictOrigin(pProjectile->m_vecOrigin(), pProjectile->GetAbsVelocity(), flLatency);

		PredictPlayers(pLocal, flLatency);
		bool bCheck = CheckEntities(pLocal, pWeapon, nullptr, pProjectile, flRadius, vOrigin);
		PredictPlayers(pLocal, 0.f);
		bCheck = bCheck && CheckEntities(pLocal, pWeapon, pCmd, pProjectile, flRadius, pProjectile->m_vecOrigin());
		RestorePlayers();
		if (bCheck)
			return true;
	}

	return false;
}

static inline bool CheckLocal(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pProjectile, float flRadius, Vec3 vOrigin)
{
	flRadius += 1;

	Vec3 vPos; reinterpret_cast<CCollisionProperty*>(pLocal->GetCollideable())->CalcNearestPoint(vOrigin, &vPos);
	if (vOrigin.DistTo(vPos) > flRadius)
		return false;

	if (!SDK::VisPosWorld(pProjectile, pLocal, vOrigin, pLocal->GetAbsOrigin() + pLocal->m_vecViewOffset(), MASK_SHOT))
		return false;

	return true;
}

bool CAutoDetonate::CheckSelf(CTFPlayer* pLocal, EntityEnum::EntityEnum iGroup)
{
	if (!(Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::PreventSelfDamage) || !pLocal->IsAlive() || pLocal->IsAGhost())
		return false;

	auto& vProjectiles = H::Entities.GetGroup(iGroup);
	if (vProjectiles.empty())
		return false;

	float flLatency = F::Backtrack.GetReal();

	for (auto pProjectile : vProjectiles)
	{
		float flRadius = 1.f;
		CTFWeaponBase* pWeapon = nullptr;
		if (!GetRadius(iGroup, pProjectile, flRadius, pWeapon))
			continue;

		Vec3 vOrigin = SDK::PredictOrigin(pProjectile->m_vecOrigin(), pProjectile->GetAbsVelocity(), flLatency);

		PredictPlayers(pLocal, 0.f, true);
		bool bCheck = CheckLocal(pLocal, pWeapon, pProjectile, flRadius, vOrigin)
				   && CheckLocal(pLocal, pWeapon, pProjectile, flRadius, pProjectile->m_vecOrigin());
		RestorePlayers();
		if (bCheck)
			return true;
	}

	return false;
}

void CAutoDetonate::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Aimbot::Projectile::AutoDetonate.Value)
		return;

	if ((Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::Stickies
		&& CheckDetonation(pLocal, EntityEnum::LocalStickies, Vars::Aimbot::Projectile::AutodetRadius.Value / 100, pCmd)
		&& !CheckSelf(pLocal, EntityEnum::LocalStickies))
		|| (Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::Flares
		&& CheckDetonation(pLocal, EntityEnum::LocalFlares, Vars::Aimbot::Projectile::AutodetRadius.Value / 100, pCmd))
		&& !CheckSelf(pLocal, EntityEnum::LocalFlares))
	{
		pCmd->buttons |= IN_ATTACK2;
	}
}