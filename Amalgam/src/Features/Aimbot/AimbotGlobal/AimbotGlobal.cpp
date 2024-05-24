#include "AimbotGlobal.h"

#include "../../Players/PlayerUtils.h"

void CAimbotGlobal::SortTargets(std::vector<Target_t>* targets, const ESortMethod& method)
{	// Sort by preference
	std::sort((*targets).begin(), (*targets).end(), [&](const Target_t& a, const Target_t& b) -> bool
			  {
				  switch (method)
				  {
					  case ESortMethod::FOV: return a.m_flFOVTo < b.m_flFOVTo;
					  case ESortMethod::DISTANCE: return a.m_flDistTo < b.m_flDistTo;
					  default: return false;
				  }
			  });
}

void CAimbotGlobal::SortPriority(std::vector<Target_t>* targets)
{	// Sort by priority
	std::sort((*targets).begin(), (*targets).end(), [&](const Target_t& a, const Target_t& b) -> bool
			  {
				  return a.m_nPriority > b.m_nPriority;
			  });
}

bool CAimbotGlobal::ShouldIgnore(CTFPlayer* pTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bMedigun)
{
	PlayerInfo_t pi{};
	if (!pTarget || pTarget == pLocal || pTarget->IsDormant())
		return true;
	if (!I::EngineClient->GetPlayerInfo(pTarget->entindex(), &pi))
		return true;

	if (pLocal->m_iTeamNum() == pTarget->m_iTeamNum())
	{
		if (bMedigun)
			return pTarget->IsInvisible();
		return false;
	}

	if (Vars::Aimbot::General::Ignore.Value & INVUL && pTarget->IsInvulnerable())
	{
		if (G::WeaponDefIndex != Heavy_t_TheHolidayPunch)
			return true;
	}
	if (Vars::Aimbot::General::Ignore.Value & CLOAKED && pTarget->IsInvisible())
	{
		if (pTarget->GetInvisPercentage() >= Vars::Aimbot::General::IgnoreCloakPercentage.Value)
			return true;
	}
	if (Vars::Aimbot::General::Ignore.Value & DEADRINGER && pTarget->m_bFeignDeathReady())
		return true;
	if (Vars::Aimbot::General::Ignore.Value & TAUNTING && pTarget->IsTaunting())
		return true;
	if (Vars::Aimbot::General::Ignore.Value & VACCINATOR)
	{
		switch (G::WeaponType)
		{
		case EWeaponType::HITSCAN:
			if (pTarget->IsBulletResist() && G::WeaponDefIndex != Spy_m_TheEnforcer)
				return true;
			break;
		case EWeaponType::PROJECTILE:
			if (pTarget->IsFireResist() && (pWeapon->m_iWeaponID() == TF_WEAPON_FLAMETHROWER || pWeapon->m_iWeaponID() == TF_WEAPON_FLAREGUN))
				return true;
			else if (pTarget->IsBulletResist() && pWeapon->m_iWeaponID() == TF_WEAPON_COMPOUND_BOW)
				return true;
			else if (pTarget->IsBlastResist())
				return true;
		}
	}
	if (Vars::Aimbot::General::Ignore.Value & DISGUISED && pTarget->IsDisguised())
		return true;

	if (F::PlayerUtils.IsIgnored(pi.friendsID))
		return true;

	return false;
}

int CAimbotGlobal::GetPriority(int targetIdx)
{
	return F::PlayerUtils.GetPriority(targetIdx);
}

// will not predict for projectile weapons
bool CAimbotGlobal::ValidBomb(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pBomb)
{
	if (G::WeaponType == EWeaponType::PROJECTILE)
		return false;

	Vec3 vOrigin = pBomb->m_vecOrigin();

	CBaseEntity* pEntity;
	for (CEntitySphereQuery sphere(vOrigin, 300.f);
		(pEntity = sphere.GetCurrentEntity()) != nullptr;
		sphere.NextEntity())
	{
		if (!pEntity || pEntity == pLocal || pEntity->IsPlayer() && (!pEntity->As<CTFPlayer>()->IsAlive() || pEntity->As<CTFPlayer>()->IsAGhost()) || pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
			continue;

		Vec3 vPos = {}; reinterpret_cast<CCollisionProperty*>(pEntity->GetCollideable())->CalcNearestPoint(vOrigin, &vPos);
		if (vOrigin.DistTo(vPos) > 300.f)
			continue;

		bool isPlayer = Vars::Aimbot::General::Target.Value & PLAYER && pEntity->IsPlayer();
		bool isSentry = Vars::Aimbot::General::Target.Value & SENTRY && pEntity->IsSentrygun();
		bool isDispenser = Vars::Aimbot::General::Target.Value & DISPENSER && pEntity->IsDispenser();
		bool isTeleporter = Vars::Aimbot::General::Target.Value & TELEPORTER && pEntity->IsTeleporter();
		bool isNPC = Vars::Aimbot::General::Target.Value & NPC && pEntity->IsNPC();
		if (isPlayer || isSentry || isDispenser || isTeleporter || isNPC)
		{
			if (isPlayer && ShouldIgnore(pEntity->As<CTFPlayer>(), pLocal, pWeapon))
				continue;

			if (!SDK::VisPosProjectile(pBomb, pEntity, vOrigin, isPlayer ? pEntity->m_vecOrigin() + pEntity->As<CTFPlayer>()->GetViewOffset() : pEntity->GetCenter(), MASK_SHOT))
				continue;

			return true;
		}
	}

	return false;
}