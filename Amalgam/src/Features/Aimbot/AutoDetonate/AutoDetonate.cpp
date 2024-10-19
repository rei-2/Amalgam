#include "AutoDetonate.h"

bool CAutoDetonate::CheckDetonation(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, EGroupType entityGroup, float flRadiusScale, CUserCmd* pCmd)
{
	for (auto pProjectile : H::Entities.GetGroup(entityGroup))
	{
		float flRadius = flRadiusScale;

		CTFWeaponBase* pWeapon = nullptr;
		if (entityGroup == EGroupType::MISC_LOCAL_STICKIES)
		{
			auto pExplosive = pProjectile->As<CTFGrenadePipebombProjectile>();
			if (pExplosive->m_iType() != TF_GL_MODE_REMOTE_DETONATE || I::GlobalVars->curtime < pExplosive->m_flCreationTime() + SDK::AttribHookValue(0.8f, "sticky_arm_time", pLocal))
				continue;

			flRadius *= 146.f;
			if (!pExplosive->m_bTouched())
			{
				static auto tf_grenadelauncher_livetime = U::ConVars.FindVar("tf_grenadelauncher_livetime");
				static auto tf_sticky_radius_ramp_time = U::ConVars.FindVar("tf_sticky_radius_ramp_time");
				static auto tf_sticky_airdet_radius = U::ConVars.FindVar("tf_sticky_airdet_radius");
				float flLiveTime = tf_grenadelauncher_livetime ? tf_grenadelauncher_livetime->GetFloat() : 0.8f;
				float flRampTime = tf_sticky_radius_ramp_time ? tf_sticky_radius_ramp_time->GetFloat() : 2.f;
				float flAirdetRadius = tf_sticky_airdet_radius ? tf_sticky_airdet_radius->GetFloat() : 0.85f;
				flRadius *= Math::RemapValClamped(I::GlobalVars->curtime - pExplosive->m_flCreationTime(), flLiveTime, flLiveTime + flRampTime, flAirdetRadius, 1.f);
			}

			pWeapon = pProjectile->As<CTFGrenadePipebombProjectile>()->m_hOriginalLauncher().Get()->As<CTFWeaponBase>();
		}
		else
		{
			flRadius *= 110.f;

			pWeapon = pProjectile->As<CTFProjectile_Flare>()->m_hLauncher().Get()->As<CTFWeaponBase>();
		}
		if (pWeapon)
			flRadius = SDK::AttribHookValue(flRadius, "mult_explosion_radius", pWeapon);

		const Vec3 vOrigin = pProjectile->m_vecOrigin();
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

			const bool isPlayer = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players && pEntity->IsPlayer() && !F::AimbotGlobal.ShouldIgnore(pEntity->As<CTFPlayer>(), pLocal, pWeapon);
			const bool isSentry = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Sentry && pEntity->IsSentrygun();
			const bool isDispenser = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Dispenser && pEntity->IsDispenser();
			const bool isTeleporter = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Teleporter && pEntity->IsTeleporter();
			const bool isSticky = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Stickies && pEntity->GetClassID() == ETFClassID::CTFGrenadePipebombProjectile && (pWeapon->m_iItemDefinitionIndex() == Demoman_s_TheQuickiebombLauncher || pWeapon->m_iItemDefinitionIndex() == Demoman_s_TheScottishResistance);
			const bool isNPC = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs && pEntity->IsNPC();
			const bool isBomb = Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Bombs && pEntity->IsBomb();
			if (isPlayer || isSentry || isDispenser || isTeleporter || isNPC || isBomb || isSticky)
			{
				if (!SDK::VisPosProjectile(pProjectile, pEntity, vOrigin, isPlayer ? pEntity->m_vecOrigin() + pEntity->As<CTFPlayer>()->GetViewOffset() : pEntity->GetCenter(), MASK_SHOT))
					continue;

				if (pWeapon->m_iItemDefinitionIndex() == Demoman_s_TheScottishResistance)
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

	if ((Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::Stickies && CheckDetonation(pLocal, pWeapon, EGroupType::MISC_LOCAL_STICKIES, Vars::Aimbot::Projectile::AutodetRadius.Value / 100, pCmd))
		|| (Vars::Aimbot::Projectile::AutoDetonate.Value & Vars::Aimbot::Projectile::AutoDetonateEnum::Flares && CheckDetonation(pLocal, pWeapon, EGroupType::MISC_LOCAL_FLARES, Vars::Aimbot::Projectile::AutodetRadius.Value / 100, pCmd)))
		pCmd->buttons |= IN_ATTACK2;
}
