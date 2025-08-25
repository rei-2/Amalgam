#include "../SDK/SDK.h"

#include "../Features/CritHack/CritHack.h"

MAKE_SIGNATURE(CTFWeaponBase_CanFireRandomCriticalShot, "client.dll", "F3 0F 58 0D ? ? ? ? 0F 2F 89", 0x0);

MAKE_HOOK(CTFWeaponBase_CanFireRandomCriticalShot, S::CTFWeaponBase_CanFireRandomCriticalShot(), bool,
	void* rcx, float flCritChance)
{	// not present on the client so it will always be a crit behind otherwise
	int nRandomRangedCritDamage = F::CritHack.GetCritDamage();
	int nTotalDamage = F::CritHack.GetRangedDamage();
	if (!nTotalDamage)
		return true;

	auto pWeapon = reinterpret_cast<CTFWeaponBase*>(rcx);
	float flNormalizedDamage = nRandomRangedCritDamage / TF_DAMAGE_CRIT_MULTIPLIER;
	pWeapon->m_flObservedCritChance() = flNormalizedDamage / (flNormalizedDamage + (nTotalDamage - nRandomRangedCritDamage));

	return CALL_ORIGINAL(rcx, flCritChance);
}