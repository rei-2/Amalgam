#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFRocketLauncher_CheckReloadMisfire, "client.dll", "48 83 EC ? E8 ? ? ? ? 32 C0", 0x0);
MAKE_SIGNATURE(CTFRocketLauncher_FireProjectile, "client.dll", "48 89 74 24 ? 57 48 81 EC ? ? ? ? 48 8B FA 48 89 9C 24", 0x0);

MAKE_HOOK(CTFRocketLauncher_CheckReloadMisfire, S::CTFRocketLauncher_CheckReloadMisfire(), bool,
	void* rcx)
{
	auto pWeapon = reinterpret_cast<CTFWeaponBase*>(rcx);
	if (!SDK::AttribHookValue(0, "can_overload", pWeapon))
		return false;

	auto pPlayer = pWeapon->m_hOwner().Get()->As<CTFPlayer>();
	int iClip1 = pWeapon->m_iClip1();
	if (pWeapon->m_bRemoveable()) // just using this var since it's in the datamap and doesn't seem to be used on the client
	{
		if (iClip1 > 0)
		{
			pWeapon->CalcIsAttackCritical();
			return true;
		}
		else
			pWeapon->m_bRemoveable() = false;
	}
	else if (iClip1 >= pWeapon->GetMaxClip1() || iClip1 > 0 && pPlayer && pPlayer->GetAmmoCount(pWeapon->m_iPrimaryAmmoType()) == 0)
	{
		pWeapon->CalcIsAttackCritical();
		pWeapon->m_bRemoveable() = true;
		return true;
	}

	return false;
}

MAKE_HOOK(CTFRocketLauncher_FireProjectile, S::CTFRocketLauncher_FireProjectile(), CBaseEntity*,
	void* rcx, CTFPlayer* pPlayer)
{
	auto pWeapon = reinterpret_cast<CTFWeaponBase*>(rcx);
	pWeapon->m_bRemoveable() = false;
	return CALL_ORIGINAL(rcx, pPlayer);
}

// more accurate way to track this? either way doesn't matter too much
/*
MAKE_SIGNATURE(CTFBat_Wood_LaunchBall, "client.dll", "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 85 C0 74 ? 48 8B CB 48 83 C4 ? 5B E9 ? ? ? ? 48 83 C4 ? 5B C3 CC CC CC CC CC CC CC CC CC CC 48 89 5C 24", 0x0);

MAKE_HOOK(CTFBat_Wood_LaunchBall, S::CTFBat_Wood_LaunchBall(), void,
	void* rcx)
{
	auto pWeapon = reinterpret_cast<CTFWeaponBase*>(rcx);
	pWeapon->CalcIsAttackCritical();

	return CALL_ORIGINAL(rcx);
}
*/