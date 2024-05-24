#pragma once
#include "../../../SDK/SDK.h"
#include "../../Backtrack/Backtrack.h"

enum struct ETargetType
{
	UNKNOWN,
	PLAYER,
	SENTRY,
	DISPENSER,
	TELEPORTER,
	STICKY,
	NPC,
	BOMBS
};

enum struct ESortMethod
{
	FOV,
	DISTANCE
};

enum Target
{
	PLAYER = 1 << 0,
	SENTRY = 1 << 1,
	DISPENSER = 1 << 2,
	TELEPORTER = 1 << 3,
	STICKY = 1 << 4,
	NPC = 1 << 5,
	BOMB = 1 << 6
};

enum Ignored
{
	INVUL = 1 << 0,
	CLOAKED = 1 << 1,
	DEADRINGER = 1 << 2,
	VACCINATOR = 1 << 3,
	UNSIMULATED = 1 << 4,
	DISGUISED = 1 << 5,
	TAUNTING = 1 << 6
};

struct Target_t
{
	CBaseEntity* m_pEntity = nullptr;
	ETargetType m_TargetType = ETargetType::UNKNOWN;
	Vec3 m_vPos = {};
	Vec3 m_vAngleTo = {};
	float m_flFOVTo = std::numeric_limits<float>::max();
	float m_flDistTo = std::numeric_limits<float>::max();
	int m_nPriority = 0;
	int m_nAimedHitbox = -1;

	TickRecord m_Tick = {};
	bool m_bBacktrack = false;
};

class CAimbotGlobal
{
public:
	void SortTargets(std::vector<Target_t>*, const ESortMethod& method);
	void SortPriority(std::vector<Target_t>*);
	bool ShouldIgnore(CTFPlayer* pTarget, CTFPlayer* pLocal, CTFWeaponBase* pWeapon, bool bMedigun = false);
	int GetPriority(int targetIdx);
	bool ValidBomb(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CBaseEntity* pBomb);
};

ADD_FEATURE(CAimbotGlobal, AimbotGlobal)