#include "../SDK/SDK.h"
#include "../Features/CritHack/CritHack.h"

MAKE_SIGNATURE(CTFWeaponBase_CalcIsAttackCritical, "client.dll", "48 89 74 24 ? 57 48 83 EC ? 48 8B F9 E8 ? ? ? ? 48 8B C8 C7 44 24 ? ? ? ? ? 4C 8D 0D ? ? ? ? 33 D2 4C 8D 05 ? ? ? ? E8 ? ? ? ? 48 8B F0 48 85 C0 0F 84 ? ? ? ? 48 8B 10", 0x0);

MAKE_HOOK(CTFWeaponBase_CalcIsAttackCritical, S::CTFWeaponBase_CalcIsAttackCritical(), void, __fastcall,
	void* ecx)
{
	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();
	if (!pLocal || !pWeapon || pWeapon != ecx)
		CALL_ORIGINAL(ecx);

	if (!F::CritHack.CalcIsAttackCriticalHandler(pLocal, pWeapon))
		return;

	const auto nPreviousWeaponMode = pWeapon->m_iWeaponMode();
	pWeapon->m_iWeaponMode() = 0;
	CALL_ORIGINAL(ecx);
	pWeapon->m_iWeaponMode() = nPreviousWeaponMode;
}