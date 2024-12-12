#include "AimbotGlobal.h"

#include "../../Players/PlayerUtils.h"

void CAimbotGlobal::SortTargets(std::vector<Target_t>* targets, int iMethod)
{	// Sort by preference
	std::sort(targets->begin(), targets->end(), [&](const Target_t& a, const Target_t& b) -> bool
		{
			switch (iMethod)
			{
			case Vars::Aimbot::General::TargetSelectionEnum::FOV: return a.m_flFOVTo < b.m_flFOVTo;
			case Vars::Aimbot::General::TargetSelectionEnum::Distance: return a.m_flDistTo < b.m_flDistTo;
			default: return false;
			}
		});
}

void CAimbotGlobal::SortPriority(std::vector<Target_t>* targets)
{	// Sort by priority
	std::sort(targets->begin(), targets->end(), [&](const Target_t& a, const Target_t& b) -> bool
		{
			return a.m_nPriority > b.m_nPriority;
		});
}

// this won't prevent shooting bones outside of fov
bool CAimbotGlobal::PlayerBoneInFOV(CTFPlayer* pTarget, Vec3 vLocalPos, Vec3 vLocalAngles, float& flFOVTo, Vec3& vPos, Vec3& vAngleTo, int iHitboxes)
{
	float flMinFOV = 180.f;
	for (int nHitbox = 0; nHitbox < pTarget->GetNumOfHitboxes(); nHitbox++)
	{
		if (!IsHitboxValid(nHitbox, iHitboxes))
			continue;

		Vec3 vCurPos = pTarget->GetHitboxCenter(nHitbox);
		Vec3 vCurAngleTo = Math::CalcAngle(vLocalPos, vCurPos);
		float flCurFOVTo = Math::CalcFov(vLocalAngles, vCurAngleTo);

		if (flCurFOVTo < flMinFOV)
		{
			vPos = vCurPos;
			vAngleTo = vCurAngleTo;
			flFOVTo = flMinFOV = flCurFOVTo;
		}
	}

	return flMinFOV < Vars::Aimbot::General::AimFOV.Value;
}

bool CAimbotGlobal::IsHitboxValid(int nHitbox, int iHitboxes)
{
	switch (nHitbox)
	{
	case -1: return true;
	case HITBOX_HEAD: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Head;
	case HITBOX_PELVIS: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Pelvis;
	case HITBOX_BODY:
	case HITBOX_THORAX:
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Body;
	case HITBOX_RIGHT_HAND:
	case HITBOX_LEFT_HAND:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_FOREARM: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Arms;
	case HITBOX_RIGHT_THIGH:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_CALF:
	case HITBOX_LEFT_CALF:
	case HITBOX_RIGHT_FOOT:
	case HITBOX_LEFT_FOOT: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Legs;
	}

	return false;
}

bool CAimbotGlobal::ShouldIgnore(CBaseEntity* pEntity, CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (pEntity->IsDormant())
		return true;

	if (auto pGameRules = I::TFGameRules->Get())
	{
		if (pGameRules->m_bTruceActive() && pLocal->m_iTeamNum() != pEntity->m_iTeamNum())
			return true;
	}

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CTFPlayer:
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer == pLocal || !pPlayer->IsAlive() || pPlayer->IsAGhost())
			return true;

		if (pLocal->m_iTeamNum() == pEntity->m_iTeamNum())
			return false;

		if (F::PlayerUtils.IsIgnored(pPlayer->entindex()))
			return true;

		if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Friends && H::Entities.IsFriend(pPlayer->entindex())
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Party && H::Entities.InParty(pPlayer->entindex())
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Invulnerable && pPlayer->IsInvulnerable() && pWeapon->m_iItemDefinitionIndex() != Heavy_t_TheHolidayPunch
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Cloaked && pPlayer->IsInvisible() && pPlayer->GetInvisPercentage() >= Vars::Aimbot::General::IgnoreCloakPercentage.Value
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::DeadRinger && pPlayer->m_bFeignDeathReady()
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Taunting && pPlayer->IsTaunting()
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Disguised && pPlayer->IsDisguised())
			return true;
		if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Vaccinator)
		{
			switch (G::PrimaryWeaponType)
			{
			case EWeaponType::HITSCAN:
				if (pPlayer->IsBulletResist() && pWeapon->m_iItemDefinitionIndex() != Spy_m_TheEnforcer)
					return true;
				break;
			case EWeaponType::PROJECTILE:
				switch (pWeapon->GetWeaponID())
				{
				case TF_WEAPON_FLAMETHROWER:
				case TF_WEAPON_FLAREGUN:
					if (pPlayer->IsFireResist())
						return true;
					break;
				case TF_WEAPON_COMPOUND_BOW:
					if (pPlayer->IsBulletResist())
						return true;
					break;
				default:
					if (pPlayer->IsBlastResist())
						return true;
				}
			}
		}

		return false;
	}
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	{
		auto pBuilding = pEntity->As<CBaseObject>();

		if (pLocal->m_iTeamNum() == pEntity->m_iTeamNum())
			return false;

		auto pOwner = pBuilding->m_hBuilder().Get();
		if (pOwner && F::PlayerUtils.IsIgnored(pOwner->entindex()))
			return true;

		if (!(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Sentry) && pBuilding->IsSentrygun()
			|| !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Dispenser) && pBuilding->IsDispenser()
			|| !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Teleporter) && pBuilding->IsTeleporter())
			return true;

		return false;
	}
	case ETFClassID::CTFGrenadePipebombProjectile:
	{
		auto pProjectile = pEntity->As<CTFGrenadePipebombProjectile>();

		if (pLocal->m_iTeamNum() == pEntity->m_iTeamNum())
			return true;

		auto pOwner = pProjectile->m_hThrower().Get();
		if (pOwner && F::PlayerUtils.IsIgnored(pOwner->entindex()))
			return true;

		if (pProjectile->m_iType() != TF_GL_MODE_REMOTE_DETONATE || !pProjectile->m_bTouched())
			return true;

		return false;
	}
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CTFTankBoss:
	case ETFClassID::CMerasmus:
	case ETFClassID::CEyeballBoss:
	{
		if (pEntity->m_iTeamNum() != 5)
			return true;

		return false;
	}
	case ETFClassID::CZombie:
	{
		if (pLocal->m_iTeamNum() == pEntity->m_iTeamNum())
			return true;

		return false;
	}
	}

	return true;
}

int CAimbotGlobal::GetPriority(int targetIdx)
{
	return F::PlayerUtils.GetPriority(targetIdx);
}

// will not predict for projectile weapons
bool CAimbotGlobal::ValidBomb(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pBomb)
{
	if (G::PrimaryWeaponType == EWeaponType::PROJECTILE)
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

		bool isPlayer = pEntity->IsPlayer() && Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Players;
		bool isSentry = pEntity->IsSentrygun() && Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Sentry;
		bool isDispenser = pEntity->IsDispenser() && Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Dispenser;
		bool isTeleporter = pEntity->IsTeleporter() && Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Teleporter;
		bool isNPC = pEntity->IsNPC() && Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs;
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