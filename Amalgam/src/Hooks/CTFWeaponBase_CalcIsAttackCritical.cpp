#include "../SDK/SDK.h"

#include "../Features/CritHack/CritHack.h"

MAKE_HOOK(CTFWeaponBase_CalcIsAttackCritical, S::CTFWeaponBase_CalcIsAttackCritical(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFWeaponBase_CalcIsAttackCritical[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	auto pWeapon = H::Entities.GetWeapon();
	if (pWeapon != rcx)
		CALL_ORIGINAL(rcx);
	else if (F::CritHack.CalcIsAttackCriticalHandler())
	{
		const auto nPreviousWeaponMode = pWeapon->m_iWeaponMode();
		pWeapon->m_iWeaponMode() = TF_WEAPON_PRIMARY_MODE;
		CALL_ORIGINAL(rcx);
		pWeapon->m_iWeaponMode() = nPreviousWeaponMode;
	}
}