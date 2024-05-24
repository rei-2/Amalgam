#include "AutoDetonate.h"

bool CAutoDetonate::CheckDetonation(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, EGroupType entityGroup, float flRadiusScale, CUserCmd* pCmd)
{
	for (auto pEntity : H::Entities.GetGroup(entityGroup))
	{
		auto pExplosive = pEntity->As<CTFGrenadePipebombProjectile>();
		if (pExplosive->m_iType() == TF_GL_MODE_REMOTE_DETONATE_PRACTICE || !pExplosive->m_bPulsed())
			continue;

		const Vec3 vOrigin = pExplosive->GetCenter();
		if (entityGroup == EGroupType::MISC_LOCAL_STICKIES)
		{
			if (pExplosive->m_iType() == TF_GL_MODE_REMOTE_DETONATE && !pExplosive->m_bTouched())
			{
				static auto tf_grenadelauncher_livetime = U::ConVars.FindVar("tf_grenadelauncher_livetime");
				static auto tf_sticky_radius_ramp_time = U::ConVars.FindVar("tf_sticky_radius_ramp_time");
				static auto tf_sticky_airdet_radius = U::ConVars.FindVar("tf_sticky_airdet_radius");
				float flLiveTime = tf_grenadelauncher_livetime ? tf_grenadelauncher_livetime->GetFloat() : 0.8f;
				float flRampTime = tf_sticky_radius_ramp_time ? tf_sticky_radius_ramp_time->GetFloat() : 2.f;
				float flAirdetRadius = tf_sticky_airdet_radius ? tf_sticky_airdet_radius->GetFloat() : 0.85f;
				flRadiusScale *= Math::RemapValClamped(I::GlobalVars->curtime - pExplosive->m_flCreationTime(), flLiveTime, flLiveTime + flRampTime, flAirdetRadius, 1.f);
			}
		}
		float flRadius = (entityGroup == EGroupType::MISC_LOCAL_STICKIES ? 146.f : 110.f) * flRadiusScale;

		// Iterate through entities in sphere radius
		CBaseEntity* pEntity;
		for (CEntitySphereQuery sphere(vOrigin, flRadius);
			(pEntity = sphere.GetCurrentEntity()) != nullptr;
			sphere.NextEntity())
		{
			if (!pEntity || pEntity == pLocal || pEntity->IsPlayer() && (!pEntity->As<CTFPlayer>()->IsAlive() || pEntity->As<CTFPlayer>()->IsAGhost()) || pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			// CEntitySphereQuery actually does a box test so we need to make sure the distance is less than the radius first
			Vec3 vPos = {}; reinterpret_cast<CCollisionProperty*>(pEntity->GetCollideable())->CalcNearestPoint(vOrigin, &vPos);
			if (vOrigin.DistTo(vPos) > flRadius)
				continue;

			const bool isPlayer = pEntity->IsPlayer() && Vars::Aimbot::General::Target.Value & PLAYER && !F::AimbotGlobal.ShouldIgnore(pEntity->As<CTFPlayer>(), pLocal, pWeapon);
			const bool isSentry = Vars::Aimbot::General::Target.Value & SENTRY && pEntity->IsSentrygun();
			const bool isDispenser = Vars::Aimbot::General::Target.Value & DISPENSER && pEntity->IsDispenser();
			const bool isTeleporter = Vars::Aimbot::General::Target.Value & TELEPORTER && pEntity->IsTeleporter();
			const bool isSticky = Vars::Aimbot::General::Target.Value & STICKY && pEntity->GetClassID() == ETFClassID::CTFGrenadePipebombProjectile && pEntity->As<CTFGrenadePipebombProjectile>()->m_iType() == TF_GL_MODE_REMOTE_DETONATE && (G::WeaponDefIndex == Demoman_s_TheQuickiebombLauncher || G::WeaponDefIndex == Demoman_s_TheScottishResistance);
			const bool isNPC = Vars::Aimbot::General::Target.Value & NPC && pEntity->IsNPC();
			const bool isBomb = Vars::Aimbot::General::Target.Value & BOMB && pEntity->IsBomb();
			if (isPlayer || isSentry || isDispenser || isTeleporter || isNPC || isBomb || isSticky)
			{
				if (!SDK::VisPosProjectile(pExplosive, pEntity, vOrigin, isPlayer ? pEntity->m_vecOrigin() + pEntity->As<CTFPlayer>()->GetViewOffset() : pEntity->GetCenter(), MASK_SHOT))
					continue;

				if (G::WeaponDefIndex == Demoman_s_TheScottishResistance)
				{
					Vec3 vAngleTo = Math::CalcAngle(pLocal->GetShootPos(), vOrigin);
					SDK::FixMovement(pCmd, vAngleTo);
					pCmd->viewangles = vAngleTo;
					G::PSilentAngles = true;
				}
				return true;
			}
		}
	}

	return false;
}

void CAutoDetonate::Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Aimbot::Projectile::AutoDetonate.Value)
		return;

	// Check sticky detonation
	if (Vars::Aimbot::Projectile::AutoDetonate.Value & (1 << 0) && CheckDetonation(pLocal, pWeapon, EGroupType::MISC_LOCAL_STICKIES, Vars::Aimbot::Projectile::AutodetRadius.Value / 100, pCmd))
		pCmd->buttons |= IN_ATTACK2;

	// Check flare detonation
	if (Vars::Aimbot::Projectile::AutoDetonate.Value & (1 << 1) && CheckDetonation(pLocal, pWeapon, EGroupType::MISC_LOCAL_FLARES, Vars::Aimbot::Projectile::AutodetRadius.Value / 100, pCmd))
		pCmd->buttons |= IN_ATTACK2;
}
