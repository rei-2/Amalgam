#include "AutoAirblast.h"

bool CAutoAirblast::CanAirblastEntity(CTFPlayer* pLocal, CBaseEntity* pEntity, Vec3& vAngle, Vec3& vPos)
{
	Vec3 vForward = {}; Math::AngleVectors(vAngle, &vForward);
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
	if (!Vars::Aimbot::Projectile::AutoAirblast.Value || !G::CanSecondaryAttack /*|| Vars::Auto::Airblast::DisableOnAttack.Value && pCmd->buttons & IN_ATTACK*/)
		return;

	const int iWeaponID = pWeapon->m_iWeaponID();
	if (iWeaponID != TF_WEAPON_FLAMETHROWER && iWeaponID != TF_WEAPON_FLAME_BALL || G::WeaponDefIndex == Pyro_m_ThePhlogistinator)
		return;

	const Vec3 vEyePos = pLocal->GetShootPos();
	bool bShouldBlast = false;

	for (auto pProjectile : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
	{
		if (pProjectile->m_iTeamNum() == pLocal->m_iTeamNum())
			continue;

		switch (pProjectile->GetClassID())
		{
		case ETFClassID::CTFGrenadePipebombProjectile:
		case ETFClassID::CTFStunBall:
		{
			if (pProjectile->As<CTFGrenadePipebombProjectile>()->m_bTouched())
				continue; // Ignore landed stickies and sandman balls
			break;
		}
		case ETFClassID::CTFProjectile_Arrow:
		{
			if (pProjectile->GetAbsVelocity().IsZero())
				continue; // Ignore arrows with no velocity / not moving
		}
		}

		Vec3 vPos = pProjectile->m_vecOrigin();
		if (Math::GetFov(I::EngineClient->GetViewAngles(), vEyePos, vPos) > Vars::Aimbot::General::AimFOV.Value)
			continue;

		if (CanAirblastEntity(pLocal, pProjectile, pCmd->viewangles, vPos))
		{
			bShouldBlast = true;
			break;
		}
		if (!bShouldBlast && Vars::Aimbot::Projectile::AutoAirblast.Value == 2) // possibly implement proj aimbot somehow ?
		{
			Vec3 vAngle = Math::CalcAngle(vEyePos, vPos);
			if (CanAirblastEntity(pLocal, pProjectile, vAngle, vPos))
			{
				SDK::FixMovement(pCmd, vAngle);
				pCmd->viewangles = vAngle;
				G::PSilentAngles = true;
				bShouldBlast = true;
				break;
			}
		}
	}

	/*
	if (!bShouldBlast && Vars::Auto::Airblast::ExtinguishPlayers.Value)
	{
		for (auto pPlayer : H::Entities.GetGroup(EGroupType::PLAYERS_TEAMMATES))
		{
			if (!pPlayer->IsOnFire() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
				continue;

			Vec3 vPos = pPlayer->m_vecOrigin() + pPlayer->GetViewOffset(); // this seems to like to overpredict ?
			if (Math::GetFov(I::EngineClient->GetViewAngles(), vEyePos, vPos) > Vars::Aimbot::General::AimFOV.Value)
				continue;

			if (CanAirblastEntity(pLocal, pPlayer, pCmd->viewangles, vPos))
			{
				bShouldBlast = true;
				break;
			}
			if (!bShouldBlast && Vars::Aimbot::Projectile::AutoAirblast.Value == 2)
			{
				Vec3 vAngle = Math::CalcAngle(vEyePos, pPlayer->GetCenter());
				if (CanAirblastEntity(pLocal, pPlayer, vAngle, vPos))
				{
					SDK::FixMovement(pCmd, vAngle);
					pCmd->viewangles = vAngle;
					G::PSilentAngles = true;
					bShouldBlast = true;
					break;
				}
			}
		}
	}
	*/

	if (bShouldBlast)
	{
		G::IsAttacking = true;
		pCmd->buttons |= IN_ATTACK2;
	}
}