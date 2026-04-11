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

		pPlayer->SetAbsOrigin(SDK::PredictOrigin(pPlayer->m_vecOrigin(), pPlayer->m_vecVelocity(), flLatency, true, pPlayer->m_vecMins() + PLAYER_ORIGIN_COMPRESSION, pPlayer->m_vecMaxs() - PLAYER_ORIGIN_COMPRESSION, pPlayer->SolidMask()));
	}
}

void CAutoDetonate::RestorePlayers()
{
	for (auto& [pEntity, vRestore] : m_mRestore)
		pEntity->SetAbsOrigin(vRestore);
	m_mRestore.clear();
}

bool CAutoDetonate::GetRadius(EntityEnum::EntityEnum eGroup, CBaseEntity* pProjectile, float& flRadius, CTFWeaponBase*& pWeapon)
{
	if (eGroup == EntityEnum::LocalStickies)
		pWeapon = pProjectile->As<CTFGrenadePipebombProjectile>()->m_hOriginalLauncher()->As<CTFWeaponBase>();
	else
		pWeapon = pProjectile->As<CTFProjectile_Flare>()->m_hLauncher()->As<CTFWeaponBase>();
	if (!pWeapon)
	{
		pWeapon = H::Entities.GetWeapon();
		if (!pWeapon)
			return false;
	}

	if (eGroup == EntityEnum::LocalStickies)
	{
		auto pPipebomb = pProjectile->As<CTFGrenadePipebombProjectile>();
		if (!pPipebomb->m_flCreationTime() || I::GlobalVars->curtime < pPipebomb->m_flCreationTime() + SDK::AttribHookValue(0.8f, "sticky_arm_time", pWeapon))
			return false;

		flRadius *= TF_ROCKET_RADIUS;
		if (!pPipebomb->m_bTouched())
		{
			static auto tf_grenadelauncher_livetime = H::ConVars.FindVar("tf_grenadelauncher_livetime");
			static auto tf_sticky_radius_ramp_time = H::ConVars.FindVar("tf_sticky_radius_ramp_time");
			static auto tf_sticky_airdet_radius = H::ConVars.FindVar("tf_sticky_airdet_radius");
			float flLiveTime = tf_grenadelauncher_livetime->GetFloat();
			float flRampTime = tf_sticky_radius_ramp_time->GetFloat();
			float flAirdetRadius = tf_sticky_airdet_radius->GetFloat();
			flRadius *= Math::RemapVal(I::GlobalVars->curtime - pPipebomb->m_flCreationTime(), flLiveTime, flLiveTime + flRampTime, flAirdetRadius, 1.f);
		}
	}
	else
		flRadius *= TF_FLARE_DET_RADIUS;
	flRadius = SDK::AttribHookValue(flRadius, "mult_explosion_radius", pWeapon);
	return true;
}

Vec3 CAutoDetonate::GetOrigin(CBaseEntity* pProjectile, EntityEnum::EntityEnum eGroup, float flLatency)
{
	Vec3 vOrigin = SDK::PredictOrigin(pProjectile->m_vecOrigin(), pProjectile->GetAbsVelocity(), flLatency);

	if (eGroup == EntityEnum::LocalStickies)
	{	// why is this even a thing?
		CGameTrace trace = {};
		CTraceFilterWorldAndPropsOnly filter = {};

		SDK::Trace(vOrigin + Vec3(0, 0, 8), vOrigin - Vec3(0, 0, 24), MASK_SHOT_HULL, &filter, &trace);
		if (trace.fraction != 1.0)
			return trace.endpos + trace.plane.normal;
	}

	return vOrigin;
}

bool CAutoDetonate::CheckEntity(CBaseEntity* pEntity, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile, float flRadius, Vec3 vOrigin)
{
	// CEntitySphereQuery actually does a box test so we need to make sure the distance is less than the radius first
	Vec3 vPos; pEntity->m_Collision()->CalcNearestPoint(vOrigin, &vPos);
	float flRadiusSqr = powf(flRadius, 2);
	if (vOrigin.DistToSqr(vPos) > flRadiusSqr)
		return false;

	if (pEntity != pLocal
		? !SDK::VisPosCollideable(pProjectile, pEntity, vOrigin, pEntity->IsPlayer() ? pEntity->GetAbsOrigin() + pEntity->As<CTFPlayer>()->GetViewOffset() : pEntity->GetCenter(), MASK_SHOT)
		: !SDK::VisPosWorld(pProjectile, pEntity, vOrigin, pEntity->GetAbsOrigin() + pEntity->As<CTFPlayer>()->m_vecViewOffset(), MASK_SHOT))
		return false;

	if (pCmd && pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER && pWeapon->As<CTFPipebombLauncher>()->GetDetonateType() == TF_DETONATE_MODE_DOT)
	{
		if (G::Attacking == 1 || I::ClientState->chokedcommands)
			return false;

		m_vAimPos = vOrigin;
	}

	return true;
}

bool CAutoDetonate::CheckEntities(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile, float flRadius, Vec3 vOrigin)
{
	flRadius -= 1;
	
	CBaseEntity* pEntity;
	for (CEntitySphereQuery sphere(vOrigin, flRadius);
		pEntity = sphere.GetCurrentEntity();
		sphere.NextEntity())
	{
		if (pEntity == pLocal || pEntity->IsPlayer() && (!pEntity->As<CTFPlayer>()->IsAlive() || pEntity->As<CTFPlayer>()->IsAGhost())
			|| !F::AimbotGlobal.FriendlyFire() && pEntity->m_iTeamNum() == pLocal->m_iTeamNum()
			|| F::AimbotGlobal.ShouldIgnore(pEntity, pLocal, pWeapon))
			continue;

		if (Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::IgnoreInvisible && pEntity->IsPlayer() && pEntity->As<CTFPlayer>()->IsInvisible(Vars::Aimbot::General::IgnoreInvisible.Value / 100.f))
			continue;

		if (CheckEntity(pEntity, pLocal, pWeapon, pCmd, pProjectile, flRadius, vOrigin))
			return true;
	}

	return false;
}

bool CAutoDetonate::CheckTargets(CTFPlayer* pLocal, EntityEnum::EntityEnum eGroup, float flRadiusScale, CUserCmd* pCmd)
{
	auto& vProjectiles = H::Entities.GetGroup(eGroup);
	if (vProjectiles.empty())
		return false;

	float flLatency = F::Backtrack.GetReal();
	for (auto pProjectile : vProjectiles)
	{
		float flRadius = flRadiusScale;
		CTFWeaponBase* pWeapon = nullptr;
		if (!GetRadius(eGroup, pProjectile, flRadius, pWeapon))
			continue;

		PredictPlayers(pLocal, flLatency);
		bool bCheck = CheckEntities(pLocal, pWeapon, nullptr, pProjectile, flRadius, GetOrigin(pProjectile, eGroup, flLatency));
		PredictPlayers(pLocal, 0.f);
		bCheck &= CheckEntities(pLocal, pWeapon, pCmd, pProjectile, flRadius, GetOrigin(pProjectile, eGroup));
		RestorePlayers();
		if (bCheck)
			return true;
	}

	return false;
}

bool CAutoDetonate::CheckSelf(CTFPlayer* pLocal, EntityEnum::EntityEnum eGroup)
{
	if (!(Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::PreventSelfDamage) || !pLocal->IsAlive() || pLocal->IsAGhost() || pLocal->IsInvulnerable())
		return false;

	auto& vProjectiles = H::Entities.GetGroup(eGroup);
	if (vProjectiles.empty())
		return false;

	float flLatency = F::Backtrack.GetReal();

	for (auto pProjectile : vProjectiles)
	{
		float flRadius = 1.f;
		CTFWeaponBase* pWeapon = nullptr;
		if (!GetRadius(eGroup, pProjectile, flRadius, pWeapon))
			continue;

		PredictPlayers(pLocal, 0.f, true);
		bool bCheck = CheckEntity(pLocal, pLocal, pWeapon, nullptr, pProjectile, flRadius, GetOrigin(pProjectile, eGroup, flLatency))
				   && CheckEntity(pLocal, pLocal, pWeapon, nullptr, pProjectile, flRadius, GetOrigin(pProjectile, eGroup));
		RestorePlayers();
		if (bCheck)
			return true;
	}

	return false;
}

bool CAutoDetonate::Check(CTFPlayer* pLocal, CUserCmd* pCmd, EntityEnum::EntityEnum eGroup, int iFlag)
{
	if (!(Vars::Aimbot::Projectile::AutoDetonate.Value & iFlag))
		return false;

	return CheckTargets(pLocal, eGroup, Vars::Aimbot::Projectile::AutodetRadius.Value / 100, pCmd)
		&& !CheckSelf(pLocal, eGroup);
}

void CAutoDetonate::Run(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!Vars::Aimbot::Projectile::AutoDetonate.Value)
		return;

	m_vAimPos = std::nullopt;
	if (Check(pLocal, pCmd, EntityEnum::LocalStickies, Vars::Aimbot::Projectile::AutoDetonateEnum::Stickies)
		|| Check(pLocal, pCmd, EntityEnum::LocalFlares, Vars::Aimbot::Projectile::AutoDetonateEnum::Flares))
	{
		pCmd->buttons |= IN_ATTACK2;

		if (m_vAimPos)
		{
			Vec3 vAngleTo = Math::CalcAngle(pLocal->GetShootPos(), *m_vAimPos);
			SDK::FixMovement(pCmd, vAngleTo);
			pCmd->viewangles = vAngleTo;
			G::PSilentAngles = true;
		}
	}
}