#include "AimbotGlobal.h"

#include "../Aimbot.h"
#include "../../Players/PlayerUtils.h"
#include "../../Ticks/Ticks.h"
#include "../../EnginePrediction/EnginePrediction.h"

std::vector<Target_t> CAimbotGlobal::ManageTargets(std::vector<Target_t>(*GetTargets)(CTFPlayer* pLocal, CTFWeaponBase* pWeapon), CTFPlayer* pLocal, CTFWeaponBase* pWeapon,
	int iMethod, int iMaxTargets)
{
	auto vTargets = GetTargets(pLocal, pWeapon);
	SortTargetsPre(vTargets, iMethod);
	vTargets.resize(std::min(size_t(iMaxTargets), vTargets.size()));
	SortTargetsPost(vTargets, iMethod);
	return vTargets;
}

void CAimbotGlobal::SortTargetsPre(std::vector<Target_t>& vTargets, int iMethod)
{
	switch (iMethod)
	{
	case Vars::Aimbot::General::TargetSelectionEnum::FOV:
		return std::sort(vTargets.begin(), vTargets.end(), [&](const Target_t& a, const Target_t& b) -> bool
		{
			return a.m_flFOVTo < b.m_flFOVTo;
		});
	case Vars::Aimbot::General::TargetSelectionEnum::Distance:
	case Vars::Aimbot::General::TargetSelectionEnum::Hybrid:
		return std::sort(vTargets.begin(), vTargets.end(), [&](const Target_t& a, const Target_t& b) -> bool
			{
				return a.m_flDistTo < b.m_flDistTo;
			});
	}
}

void CAimbotGlobal::SortTargetsPost(std::vector<Target_t>& vTargets, int iMethod)
{
	switch (iMethod)
	{
	case Vars::Aimbot::General::TargetSelectionEnum::Hybrid:
		return std::sort(vTargets.begin(), vTargets.end(), [&](const Target_t& a, const Target_t& b) -> bool
			{
				return a.m_flFOVTo < b.m_flFOVTo;
			});
	}

	std::sort(vTargets.begin(), vTargets.end(), [&](const Target_t& a, const Target_t& b) -> bool
		{
			return a.m_nPriority > b.m_nPriority;
		});
}

// this won't prevent shooting bones outside of fov
bool CAimbotGlobal::PlayerBoneInFOV(CTFPlayer* pTarget, Vec3 vLocalPos, Vec3 vLocalAngles, float& flFOVTo, Vec3& vPos, Vec3& vAngleTo, int iHitboxes)
{
	matrix3x4* aBones = F::Backtrack.GetBones(pTarget);
	if (!Vars::Visuals::Removals::Interpolation.Value)
	{
		std::vector<TickRecord*> vRecords = {};
		if (F::Backtrack.GetRecords(pTarget, vRecords) && !vRecords.empty())
		{
			float flLerp = Vars::Visuals::Removals::Lerp.Value ? 0.f : G::Lerp;
			for (auto pRecord : vRecords)
			{
				if (F::EnginePrediction.m_flOldCurrentTime - pRecord->m_flSimTime > flLerp)
				{
					aBones = pRecord->m_aBones;
					break;
				}
			}
		}
	}
	if (!aBones)
		return false;

	float flMinFOV = 180.f;
	for (int nHitbox = 0; nHitbox < pTarget->GetNumOfHitboxes(); nHitbox++)
	{
		if (!IsHitboxValid(pTarget, nHitbox, iHitboxes))
			continue;

		Vec3 vCurPos = pTarget->GetHitboxCenter(aBones, nHitbox);
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

bool CAimbotGlobal::IsHitboxValid(CBaseEntity* pEntity, int nHitbox, int iHitboxes)
{
	switch (pEntity->GetHitboxToBase(nHitbox))
	{
	case -1: return true;
	case HITBOX_HEAD: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Head;
	case HITBOX_SPINE0:
	case HITBOX_SPINE1:
	case HITBOX_SPINE2:
	case HITBOX_SPINE3: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Body;
	case HITBOX_PELVIS: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Pelvis;
	case HITBOX_LEFT_UPPERARM:
	case HITBOX_LEFT_FOREARM:
	case HITBOX_LEFT_HAND:
	case HITBOX_RIGHT_UPPERARM:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_RIGHT_HAND: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Arms;
	case HITBOX_LEFT_THIGH:
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_FOOT:
	case HITBOX_RIGHT_THIGH:
	case HITBOX_RIGHT_CALF:
	case HITBOX_RIGHT_FOOT: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Legs;
	}

	return false;
}

bool CAimbotGlobal::IsHitboxValid(int nHitbox, int iHitboxes)
{
	switch (nHitbox)
	{
	case BOUNDS_HEAD: return iHitboxes & Vars::Aimbot::Projectile::HitboxesEnum::Head;
	case BOUNDS_BODY: return iHitboxes & Vars::Aimbot::Projectile::HitboxesEnum::Body;
	case BOUNDS_FEET: return iHitboxes & Vars::Aimbot::Projectile::HitboxesEnum::Feet;
	}

	return false;
}

bool CAimbotGlobal::ShouldMultipoint(CBaseEntity* pEntity, int nHitbox, int iHitboxes)
{
	if (Vars::Aimbot::Hitscan::MultipointScale.Value <= 0.f)
		return false;

	if (!iHitboxes)
		return true;

	switch (pEntity->GetHitboxToBase(nHitbox))
	{
	case HITBOX_HEAD: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Head;
	case HITBOX_SPINE0:
	case HITBOX_SPINE1:
	case HITBOX_SPINE2:
	case HITBOX_SPINE3:
	case HITBOX_PELVIS: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Body;
	case HITBOX_LEFT_UPPERARM:
	case HITBOX_LEFT_FOREARM:
	case HITBOX_LEFT_HAND:
	case HITBOX_RIGHT_UPPERARM:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_RIGHT_HAND: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Arms;
	case HITBOX_LEFT_THIGH:
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_FOOT:
	case HITBOX_RIGHT_THIGH:
	case HITBOX_RIGHT_CALF:
	case HITBOX_RIGHT_FOOT: return iHitboxes & Vars::Aimbot::Hitscan::HitboxesEnum::Legs;
	}

	return false;
}

bool CAimbotGlobal::ShouldIgnore(CBaseEntity* pEntity, CTFPlayer* pLocal, CTFWeaponBase* pWeapon)
{
	if (pEntity->IsDormant())
		return true;

	if (auto pGameRules = I::TFGameRules())
	{
		if (pGameRules->m_bTruceActive() && (FriendlyFire() || pLocal->m_iTeamNum() != pEntity->m_iTeamNum()))
			return true;
	}

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CTFPlayer:
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer == pLocal || !pPlayer->IsAlive() || pPlayer->IsAGhost())
			return true;

		if (!FriendlyFire() && pLocal->m_iTeamNum() == pEntity->m_iTeamNum())
			return false;

		if (F::PlayerUtils.IsIgnored(pPlayer->entindex())
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Unprioritized && !F::PlayerUtils.IsPrioritized(pPlayer->entindex()))
			return true;

		if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Friends && H::Entities.IsFriend(pPlayer->entindex())
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Party && H::Entities.InParty(pPlayer->entindex())
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Invulnerable && pPlayer->IsInvulnerable() && SDK::AttribHookValue(0, "crit_forces_victim_to_laugh", pWeapon) <= 0
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Invisible && pPlayer->IsInvisible(Vars::Aimbot::General::IgnoreInvisible.Value / 100.f)
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::DeadRinger && pPlayer->m_bFeignDeathReady()
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Taunting && pPlayer->IsTaunting()
			|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Disguised && pPlayer->InCond(TF_COND_DISGUISED))
			return true;
		if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Vaccinator)
		{
			switch (G::PrimaryWeaponType)
			{
			case EWeaponType::HITSCAN:
				if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST) && SDK::AttribHookValue(0, "mod_pierce_resists_absorbs", pWeapon) != 0)
					return true;
				break;
			case EWeaponType::PROJECTILE:
				switch (pWeapon->GetWeaponID())
				{
				case TF_WEAPON_FLAMETHROWER:
				case TF_WEAPON_FLAREGUN:
					if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST))
						return true;
					break;
				case TF_WEAPON_COMPOUND_BOW:
					if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST))
						return true;
					break;
				default:
					if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST))
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
		if (pLocal->m_iTeamNum() == pBuilding->m_iTeamNum())
			return false;

		if (!(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Sentry) && pBuilding->IsSentrygun()
			|| !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Dispenser) && pBuilding->IsDispenser()
			|| !(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Teleporter) && pBuilding->IsTeleporter())
			return true;

		auto pOwner = pBuilding->m_hBuilder().Get();
		if (pOwner)
		{
			if (F::PlayerUtils.IsIgnored(pOwner->entindex()))
				return true;

			if (Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Friends && H::Entities.IsFriend(pOwner->entindex())
				|| Vars::Aimbot::General::Ignore.Value & Vars::Aimbot::General::IgnoreEnum::Party && H::Entities.InParty(pOwner->entindex()))
				return true;
		}

		return false;
	}
	case ETFClassID::CTFGrenadePipebombProjectile:
	{
		auto pProjectile = pEntity->As<CTFGrenadePipebombProjectile>();

		if (!(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Stickies))
			return true;

		if (pLocal->m_iTeamNum() == pEntity->m_iTeamNum())
			return true;

		auto pOwner = pProjectile->m_hThrower().Get();
		if (pOwner && F::PlayerUtils.IsIgnored(pOwner->entindex()))
			return true;

		if (pProjectile->m_iType() != TF_GL_MODE_REMOTE_DETONATE || !pProjectile->m_bTouched())
			return true;

		return false;
	}
	case ETFClassID::CTFBaseBoss:
	case ETFClassID::CTFTankBoss:
	case ETFClassID::CMerasmus:
	case ETFClassID::CEyeballBoss:
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CZombie:
	{
		if (!(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::NPCs))
			return true;

		if (pEntity->GetClassID() == ETFClassID::CEyeballBoss
			? pLocal->m_iTeamNum() != TF_TEAM_HALLOWEEN
			: pLocal->m_iTeamNum() == pEntity->m_iTeamNum())
			return true;

		return false;
	}
	case ETFClassID::CTFGenericBomb:
	case ETFClassID::CTFPumpkinBomb:
	{
		if (!(Vars::Aimbot::General::Target.Value & Vars::Aimbot::General::TargetEnum::Bombs))
			return true;

		if (!ValidBomb(pLocal, pWeapon, pEntity))
			return true;

		return false;
	}
	}

	return true;
}

int CAimbotGlobal::GetPriority(int iIndex)
{
	return F::PlayerUtils.GetPriority(iIndex);
}

bool CAimbotGlobal::FriendlyFire()
{
	static auto mp_friendlyfire = H::ConVars.FindVar("mp_friendlyfire");
	return mp_friendlyfire->GetBool();
}

bool CAimbotGlobal::ShouldAim()
{
	switch (Vars::Aimbot::General::AimType.Value)
	{
	case Vars::Aimbot::General::AimTypeEnum::Plain:
	case Vars::Aimbot::General::AimTypeEnum::Silent:
		if (!G::CanPrimaryAttack && !G::Reloading && !F::Ticks.IsTimingUnsure())
			return false;
	}

	return true;
}

bool CAimbotGlobal::ShouldHoldAttack(CTFWeaponBase* pWeapon)
{
	switch (Vars::Aimbot::General::AimHoldsFire.Value)
	{
	case Vars::Aimbot::General::AimHoldsFireEnum::MinigunOnly:
		if (pWeapon->GetWeaponID() != TF_WEAPON_MINIGUN)
			break;
		[[fallthrough]];
	case Vars::Aimbot::General::AimHoldsFireEnum::Always:
		if (!F::Aimbot.m_bRunningSecondary && !G::CanPrimaryAttack && G::LastUserCmd->buttons & IN_ATTACK && Vars::Aimbot::General::AimType.Value && !pWeapon->IsInReload())
			return true;
	}
	return false;
}

// will not predict for projectile weapons
bool CAimbotGlobal::ValidBomb(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pBomb)
{
	Vec3 vOrigin = pBomb->m_vecOrigin();

	CBaseEntity* pEntity;
	for (CEntitySphereQuery sphere(vOrigin, 300.f);
		pEntity = sphere.GetCurrentEntity();
		sphere.NextEntity())
	{
		if (pEntity == pLocal || pEntity->IsPlayer() && (!pEntity->As<CTFPlayer>()->IsAlive() || pEntity->As<CTFPlayer>()->IsAGhost())
			|| !FriendlyFire() && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
			continue;

		Vec3 vPos; pEntity->m_Collision()->CalcNearestPoint(vOrigin, &vPos);
		if (vOrigin.DistTo(vPos) > 300.f)
			continue;

		if (pEntity->IsPlayer() || pEntity->IsBuilding() || pEntity->IsNPC())
		{
			if (ShouldIgnore(pEntity, pLocal, pWeapon))
				continue;

			if (!SDK::VisPosCollideable(pBomb, pEntity, vOrigin, pEntity->IsPlayer() ? pEntity->m_vecOrigin() + pEntity->As<CTFPlayer>()->GetViewOffset() : pEntity->GetCenter(), MASK_SHOT))
				continue;

			return true;
		}
	}

	return false;
}