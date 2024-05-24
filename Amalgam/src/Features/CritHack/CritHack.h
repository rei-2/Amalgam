#pragma once
#include "../../SDK/SDK.h"

typedef int                i32;
typedef unsigned int       u32;

struct WeaponStorage
{
	float Damage = 0.f;
	float Cost = 0.f;
	int AvailableCrits = 0;
	int PotentialCrits = 0;
	int StreamWait = -1;

	int EntIndex = -1;
	int DefIndex = -1;

	std::deque<int> CritCommands = {};
	std::deque<int> SkipCommands = {};
};

class CCritHack
{
private:
	void Fill(const CUserCmd* pCmd, int n = 10);

	bool IsCritCommand(int iSlot, int iIndex, const i32 command_number, const bool bCrit = true, const bool bSafe = true);
	u32 DecryptOrEncryptSeed(int iSlot, int iIndex, const u32 uSeed);

	void GetTotalCrits(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	void CanFireCritical(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	bool WeaponCanCrit(CTFWeaponBase* pWeapon);

	void ResetWeapons(CTFPlayer* pLocal);
	void Reset();

	int CritDamage = 0.f;
	int AllDamage = 0.f;
	std::unordered_map<int, int> mHealthStorage = {};

	bool CritBanned = false;
	int DamageTilUnban = 0;
	float CritChance = 0.f;
	int WishRandomSeed = 0;

public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon, CUserCmd* pCmd);
	bool CalcIsAttackCriticalHandler(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);
	void Event(IGameEvent* pEvent, FNV1A_t uHash, CTFPlayer* pLocal);
	void Store();
	void Draw(CTFPlayer* pLocal);

	std::unordered_map<int, WeaponStorage> Storage = {};
};

ADD_FEATURE(CCritHack, CritHack)