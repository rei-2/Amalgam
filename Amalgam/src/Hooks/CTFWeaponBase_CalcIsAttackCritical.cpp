#include "../SDK/SDK.h"

#include "../Features/CritHack/CritHack.h"

MAKE_HOOK(CTFWeaponBase_CalcIsAttackCritical, S::CTFWeaponBase_CalcIsAttackCritical(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFWeaponBase_CalcIsAttackCritical[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	auto pWeapon = reinterpret_cast<CTFWeaponBase*>(rcx);

	const auto nPreviousWeaponMode = pWeapon->m_iWeaponMode();
	pWeapon->m_iWeaponMode() = TF_WEAPON_PRIMARY_MODE;
	if (I::Prediction->m_bFirstTimePredicted)
	{
		*G::RandomSeed() = F::CritHack.m_iWishRandomSeed;
		CALL_ORIGINAL(rcx);
	}
	else // fixes minigun and flamethrower buggy crit sounds for the most part
	{
		float flOldCritTokenBucket = pWeapon->m_flCritTokenBucket();
		int nOldCritChecks = pWeapon->m_nCritChecks();
		int nOldCritSeedRequests = pWeapon->m_nCritSeedRequests();
		float flOldLastRapidFireCritCheckTime = pWeapon->m_flLastRapidFireCritCheckTime();
		float flOldCritTime = pWeapon->m_flCritTime();
		CALL_ORIGINAL(rcx);
		pWeapon->m_flCritTokenBucket() = flOldCritTokenBucket;
		pWeapon->m_nCritChecks() = nOldCritChecks;
		pWeapon->m_nCritSeedRequests() = nOldCritSeedRequests;
		pWeapon->m_flLastRapidFireCritCheckTime() = flOldLastRapidFireCritCheckTime;
		pWeapon->m_flCritTime() = flOldCritTime;
	}
	pWeapon->m_iWeaponMode() = nPreviousWeaponMode;
}