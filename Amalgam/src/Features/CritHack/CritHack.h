#pragma once
#include "../../SDK/SDK.h"

typedef int                i32;
typedef unsigned int       u32;

struct WeaponStorage_t
{
	int m_iEntIndex = -1;
	int m_iDefIndex = -1;
	float m_flMultCritChance = 1.f;

	float m_flDamage = 0.f;
	float m_flCost = 0.f;
	int m_iAvailableCrits = 0;
	int m_iPotentialCrits = 0;
	int m_iNextCrit = 0;

	std::deque<int> m_vCritCommands = {};
	std::deque<int> m_vSkipCommands = {};
};

class CCritHack
{
private:
	void Fill(const CUserCmd* pCmd, int n = 10);

	bool IsCritCommand(int iSlot, int iIndex, float flMultCritChance, const i32 command_number, const bool bCrit = true, const bool bSafe = true);
	u32 DecryptOrEncryptSeed(int iSlot, int iIndex, const u32 uSeed);

	void GetTotalCrits(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	void CanFireCritical(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	bool WeaponCanCrit(CTFWeaponBase* pWeapon, bool bWeaponOnly = false);

	void ResetWeapons(CTFPlayer* pLocal);
	void Reset();

	int m_iCritDamage = 0.f;
	int m_iAllDamage = 0.f;
	std::unordered_map<int, int> m_mHealthStorage = {};

	bool m_bCritBanned = false;
	int m_iDamageTilUnban = 0;
	float m_flCritChance = 0.f;
	int m_iWishRandomSeed = 0;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	bool CalcIsAttackCriticalHandler(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	void Event(IGameEvent* pEvent, uint32_t uHash, CTFPlayer* pLocal);
	void Store();
	void Draw(CTFPlayer* pLocal);
	int PredictCmdNum(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	std::unordered_map<int, WeaponStorage_t> m_mStorage = {};
};

ADD_FEATURE(CCritHack, CritHack)