#pragma once
#include "../../SDK/SDK.h"

Enum(CritRequest, Any, Crit, Skip);

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
	int GetCritCommand(CTFWeaponBase* pWeapon, int iCommandNumber, bool bCrit = true, bool bSafe = true);
	bool IsCritCommand(int iCommandNumber, CTFWeaponBase* pWeapon, bool bCrit = true, bool bSafe = true);
	bool IsCritSeed(int iSeed, CTFWeaponBase* pWeapon, bool bCrit = true, bool bSafe = true);
	int CommandToSeed(int iCommandNumber);

	void UpdateWeaponInfo(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	void UpdateInfo(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	int GetCritRequest(CUserCmd* pCmd, CTFWeaponBase* pWeapon);

	void Reset();
	void StoreHealthHistory(int iIndex, int iHealth, bool bDamage = false);

	int m_iCritDamage = 0;
	int m_iRangedDamage = 0;
	int m_iMeleeDamage = 0;
	int m_iResourceDamage = 0;
	int m_iDesyncDamage = 0;
	std::unordered_map<int, HealthHistory_t> m_mHealthHistory = {};

	bool m_bCritBanned = false;
	float m_flDamageTilFlip = 0;

	float m_flDamage = 0.f;
	float m_flCost = 0.f;
	int m_iAvailableCrits = 0;
	int m_iPotentialCrits = 0;
	int m_iNextCrit = 0;

	int m_iEntIndex = 0;
	bool m_bMelee = false;
	float m_flCritChance = 0.f;
	float m_flMultCritChance = 1.f;

	//float m_flLastDamageTime = 0.f;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	void Event(IGameEvent* pEvent, uint32_t uHash, CTFPlayer* pLocal);
	void Store();
	void Draw(CTFPlayer* pLocal);

	bool WeaponCanCrit(CTFWeaponBase* pWeapon, bool bWeaponOnly = false);
	int PredictCmdNum(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);

	float GetCritDamage() { return m_iCritDamage; }
	float GetRangedDamage() { return m_iRangedDamage; }
};

ADD_FEATURE(CCritHack, CritHack);