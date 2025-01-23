#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBasePlayer_ItemPostFrame, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 39 48 8B F1 FF 97", 0x0);

MAKE_HOOK(CBasePlayer_ItemPostFrame, S::CBasePlayer_ItemPostFrame(), void,
	void* rcx)
{
	auto pLocal = reinterpret_cast<CTFPlayer*>(rcx);
	auto pWeapon = H::Entities.GetWeapon();
	if (!pWeapon)
		return CALL_ORIGINAL(rcx);

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_PARTICLE_CANNON:
		break;
	default:
		return CALL_ORIGINAL(rcx);
	}
	switch (pWeapon->m_iReloadMode())
	{
	case 1:
		if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka
			|| I::GlobalVars->curtime < pLocal->m_flNextAttack())
			return CALL_ORIGINAL(rcx);
		break;
	case 2:
		if (pWeapon->m_iItemDefinitionIndex() != Soldier_m_TheBeggarsBazooka
			? TIME_TO_TICKS(pLocal->m_flNextAttack() - I::GlobalVars->curtime) != 31
			: I::GlobalVars->curtime < pLocal->m_flNextAttack())
			return CALL_ORIGINAL(rcx);
		break;
	default:
		return CALL_ORIGINAL(rcx);
	}

	CALL_ORIGINAL(rcx);
	pWeapon->IncrementAmmo();
	pWeapon->m_bReloadedThroughAnimEvent() = true;
}