#include "AutoDetonate.h"

static inline Vec3 PredictOrigin(Vec3& vOrigin, Vec3 vVelocity, float flLatency, bool bTrace = true, Vec3 vMins = {}, Vec3 vMaxs = {}, unsigned int nMask = MASK_SOLID)
{
	if (vVelocity.IsZero())
		return vOrigin;

	Vec3 vTo = vOrigin + vVelocity * flLatency;
	if (!bTrace)
		return vTo;

	CGameTrace trace = {};
	CTraceFilterWorldAndPropsOnly filter = {};

	SDK::TraceHull(vOrigin, vTo, vMins, vMaxs, nMask, &filter, &trace);
	return vOrigin + (vTo - vOrigin) * trace.fraction;
}

void CAutoDetonate::PredictPlayers(CBaseEntity* pLocal, float flLatency)
{
	m_mRestore.clear();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer == pLocal || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
			continue;

		m_mRestore[pPlayer] = pPlayer->GetAbsOrigin();

		pPlayer->SetAbsOrigin(PredictOrigin(pPlayer->m_vecOrigin(), pPlayer->m_vecVelocity(), flLatency, true, pPlayer->m_vecMins() + 0.125f, pPlayer->m_vecMaxs() - 0.125f, MASK_PLAYERSOLID));
	}
}

void CAutoDetonate::RestorePlayers()
{
	for (auto& [pEntity, vRestore] : m_mRestore)
	{
		pEntity->SetAbsOrigin(vRestore);
	}
}

static inline bool CheckEntities(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd, CBaseEntity* pProjectile, float flRadius, Vec3 vOrigin)
{
	flRadius -= 1;

	CBaseEntity* pEntity;
	for (CEntitySphereQuery sphere(vOrigin, flRadius);
		(pEntity = sphere.GetCurrentEntity()) != nullptr;
		sphere.NextEntity())
	{
		if (pEntity == pLocal || pEntity->IsPlayer() && (!pEntity->As<CTFPlayer>()->IsAlive() || pEntity->As<CTFPlayer>()->IsAGhost()) || pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
			continue;

		// CEntitySphereQuery actually does a box test so we need to make sure the distance is less than the radius first
		Vec3 vPos = {}; reinterpret_cast<CCollisionProperty*>(pEntity->GetCollideable())->CalcNearestPoint(vOrigin, &vPos);
		if (vOrigin.DistTo(vPos) > flRadius)
			continue;

		bool isPlayer = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players
			&& pEntity->IsPlayer() && !F::AimbotGlobal.ShouldIgnore(pEntity->As<CTFPlayer>(), pLocal, pWeapon);
		bool isSentry = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Sentry
			&& pEntity->IsSentrygun();
		bool isDispenser = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Dispenser
			&& pEntity->IsDispenser();
		bool isTeleporter = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Teleporter
			&& pEntity->IsTeleporter();
		bool isSticky = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Stickies
			&& pEntity->GetClassID() == ETFClassID::CTFGrenadePipebombProjectile && pEntity->As<CTFGrenadePipebombProjectile>()->HasStickyEffects() && (pWeapon->m_iItemDefinitionIndex() == Demoman_s_TheQuickiebombLauncher || pWeapon->m_iItemDefinitionIndex() == Demoman_s_TheScottishResistance);
		bool isNPC = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs
			&& pEntity->IsNPC();
		bool isBomb = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Bombs
			&& pEntity->IsBomb();
		if (isPlayer || isSentry || isDispenser || isTeleporter || isNPC || isBomb || isSticky)
		{
			if (!SDK::VisPosProjectile(pProjectile, pEntity, vOrigin, isPlayer ? pEntity->GetAbsOrigin() + pEntity->As<CTFPlayer>()->GetViewOffset() : pEntity->GetCenter(), MASK_SHOT))
				continue;

			if (pCmd && pWeapon && pWeapon->m_iItemDefinitionIndex() == Demoman_s_TheScottishResistance)
			{
				Vec3 vAngleTo = Math::CalcAngle(pLocal->GetShootPos(), vOrigin);
				SDK::FixMovement(pCmd, vAngleTo);
				pCmd->viewangles = vAngleTo;
				G::PSilentAngles = true;
			}

			return true;
		}
	}

	return false;
}

bool CAutoDetonate::CheckDetonation(CTFPlayer* pLocal, EGroupType entityGroup, float flRadiusScale, CUserCmd* pCmd)
{
	auto& vProjectiles = H::Entities.GetGroup(entityGroup);
	if (vProjectiles.empty())
		return false;

	float flLatency = std::max(F::Backtrack.GetReal() - 0.05f, 0.f);

	for (auto pProjectile : vProjectiles)
	{
		float flRadius = flRadiusScale;

		CTFWeaponBase* pWeapon = nullptr;
		if (entityGroup == EGroupType::MISC_LOCAL_STICKIES)
			pWeapon = pProjectile->As<CTFGrenadePipebombProjectile>()->m_hOriginalLauncher().Get()->As<CTFWeaponBase>();
		else
			pWeapon = pProjectile->As<CTFProjectile_Flare>()->m_hLauncher().Get()->As<CTFWeaponBase>();
		if (!pWeapon)
		{
			pWeapon = H::Entities.GetWeapon();
			if (!pWeapon)
				continue;
		}

		if (entityGroup == EGroupType::MISC_LOCAL_STICKIES)
		{
			auto pPipebomb = pProjectile->As<CTFGrenadePipebombProjectile>();
			if (!pPipebomb->m_flCreationTime() || I::GlobalVars->curtime < pPipebomb->m_flCreationTime() + SDK::AttribHookValue(0.8f, "sticky_arm_time", pWeapon))
				continue;

			flRadius *= 146.f;
			if (!pPipebomb->m_bTouched())
			{
				static auto tf_grenadelauncher_livetime = U::ConVars.FindVar("tf_grenadelauncher_livetime");
				static auto tf_sticky_radius_ramp_time = U::ConVars.FindVar("tf_sticky_radius_ramp_time");
				static auto tf_sticky_airdet_radius = U::ConVars.FindVar("tf_sticky_airdet_radius");
				float flLiveTime = tf_grenadelauncher_livetime ? tf_grenadelauncher_livetime->GetFloat() : 0.8f;
				float flRampTime = tf_sticky_radius_ramp_time ? tf_sticky_radius_ramp_time->GetFloat() : 2.f;
				float flAirdetRadius = tf_sticky_airdet_radius ? tf_sticky_airdet_radius->GetFloat() : 0.85f;
				flRadius *= Math::RemapValClamped(I::GlobalVars->curtime - pPipebomb->m_flCreationTime(), flLiveTime, flLiveTime + flRampTime, flAirdetRadius, 1.f);
			}
		}
		else
			flRadius *= 110.f;
		flRadius = SDK::AttribHookValue(flRadius, "mult_explosion_radius", pWeapon);
		Vec3 vOrigin = PredictOrigin(pProjectile->m_vecOrigin(), pProjectile->GetAbsVelocity(), flLatency);

		PredictPlayers(pLocal, flLatency);
		bool bCheck = CheckEntities(pLocal, pWeapon, nullptr, pProjectile, flRadius, vOrigin);
		RestorePlayers();
		if (bCheck && CheckEntities(pLocal, pWeapon, pCmd, pProjectile, flRadius, pProjectile->m_vecOrigin()))
			return true;
	}

	return false;
}

void CAutoDetonate::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Aimbot::Projectile::AutoDetonate.Value)
		return;

	if ((Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::Stickies && CheckDetonation(pLocal, EGroupType::MISC_LOCAL_STICKIES, Vars::Aimbot::Projectile::AutodetRadius.Value / 100, pCmd))
		|| (Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::Flares && CheckDetonation(pLocal, EGroupType::MISC_LOCAL_FLARES, Vars::Aimbot::Projectile::AutodetRadius.Value / 100, pCmd)))
		pCmd->buttons |= IN_ATTACK2;
}
