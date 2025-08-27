#pragma once
#include "../../../SDK/SDK.h"
#include "../../Backtrack/Backtrack.h"

enum BOUNDS_HITBOXES
{
	BOUNDS_HEAD, BOUNDS_BODY, BOUNDS_FEET
};

Enum(Target, Unknown, Player, Sentry, Dispenser, Teleporter, Sticky, NPC, Bomb)

struct Target_t
{
	CBaseEntity* m_pEntity = nullptr;
	int m_iTargetType = TargetEnum::Unknown;
	Vec3 m_vPos = {};
	Vec3 m_vAngleTo = {};
	float m_flFOVTo = std::numeric_limits<float>::max();
	float m_flDistTo = std::numeric_limits<float>::max();
	int m_nPriority = 0;
	int m_nAimedHitbox = -1;

	TickRecord* m_pRecord = nullptr;
	bool m_bBacktrack = false;
};

class CAimbotGlobal
{
public:
	void SortTargets(std::vector<Target_t>&, int iMethod);
	void SortPriority(std::vector<Target_t>&);

	bool PlayerBoneInFOV(CTFPlayer* pTarget, Vec3 vLocalPos, Vec3 vLocalAngles, float& flFOVTo, Vec3& vPos, Vec3& vAngleTo, int iHitboxes = Vars::Aimbot::Hitscan::HitboxesEnum::Head | Vars::Aimbot::Hitscan::HitboxesEnum::Body | Vars::Aimbot::Hitscan::HitboxesEnum::Pelvis | Vars::Aimbot::Hitscan::HitboxesEnum::Arms | Vars::Aimbot::Hitscan::HitboxesEnum::Legs);
	bool IsHitboxValid(uint32_t uHash, int nHitbox, int iHitboxes = Vars::Aimbot::Hitscan::HitboxesEnum::Head | Vars::Aimbot::Hitscan::HitboxesEnum::Body | Vars::Aimbot::Hitscan::HitboxesEnum::Pelvis | Vars::Aimbot::Hitscan::HitboxesEnum::Arms | Vars::Aimbot::Hitscan::HitboxesEnum::Legs);
	bool IsHitboxValid(int nHitbox, int iHitboxes = Vars::Aimbot::Projectile::HitboxesEnum::Head | Vars::Aimbot::Projectile::HitboxesEnum::Body | Vars::Aimbot::Projectile::HitboxesEnum::Feet);

	bool ShouldIgnore(CBaseEntity* pTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	int GetPriority(int iIndex);
	bool FriendlyFire();

	bool ShouldAim();
	bool ShouldHoldAttack(CTFWeaponBase* pWeapon);
	bool ValidBomb(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pBomb);
};

ADD_FEATURE(CAimbotGlobal, AimbotGlobal);