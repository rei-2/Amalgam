#pragma once
#include "../../SDK/SDK.h"

typedef int                i32;
typedef unsigned int       u32;

struct WeaponStorage_t
{
	int m_iEntIndex = -1;
	int m_iDefIndex = -1;
	float m_flMultCritChance = 1.f;

	float m_flDamage = -1.f;
	float m_flCost = 0.f;
	int m_iAvailableCrits = 0;
	int m_iPotentialCrits = 0;
	int m_iNextCrit = 0;

	std::deque<int> m_vCritCommands = {};
	std::deque<int> m_vSkipCommands = {};

	bool m_bActive = false;
};

struct HealthHistory_t
{
	int m_iNewHealth = 0;
	int m_iOldHealth = 0;

	struct HealthStorage_t
	{
		int m_iOldHealth = 0;
		float m_flTime = 0.f;
	};
	std::unordered_map<int, HealthStorage_t> m_mHistory = {};
};

class CCritHack
{
private:
	void Fill(const CUserCmd* pCmd, int n = 10);

	bool IsCritCommand(int iSlot, int iIndex, float flMultCritChance, const i32 command_number, const bool bCrit = true, const bool bSafe = true);
	u32 DecryptOrEncryptSeed(int iSlot, int iIndex, const u32 uSeed);

	void GetTotalCrits(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	void CanFireCritical(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	void ResetWeapons(CTFPlayer* pLocal);
	void Reset();

	void StoreHealthHistory(int iIndex, int iHealth, bool bDamage = false);

	int m_iFillStart = 0;

	int m_iCritDamage = 0;
	int m_iRangedDamage = 0;
	std::unordered_map<int, HealthHistory_t> m_mHealthHistory = {};

	int m_iMeleeDamage = 0;
	int m_iResourceDamage = 0;
	int m_iDesyncDamage = 0;

	bool m_bCritBanned = false;
	float m_flDamageTilFlip = 0;
	float m_flCritChance = 0.f;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void Event(IGameEvent* pEvent, uint32_t uHash, CTFPlayer* pLocal);
	void Store();
	void Draw(CTFPlayer* pLocal);

	bool WeaponCanCrit(CTFWeaponBase* pWeapon, bool bWeaponOnly = false);
	int PredictCmdNum(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	std::unordered_map<int, WeaponStorage_t> m_mStorage = {};
	int m_iWishRandomSeed = 0;
};

ADD_FEATURE(CCritHack, CritHack);