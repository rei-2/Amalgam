#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFWeaponBase_GetShootSound, "client.dll", "40 55 56 41 56 48 83 EC ? 80 B9", 0x0);

MAKE_HOOK(CTFWeaponBase_GetShootSound, S::CTFWeaponBase_GetShootSound(), const char*,
	void* rcx, int iIndex)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFWeaponBase_GetShootSound.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, iIndex);
#endif

	if (Vars::Misc::Sound::GiantWeaponSounds.Value)
	{
		auto pWeapon = reinterpret_cast<CBaseCombatWeapon*>(rcx);
		auto pOwner = pWeapon ? I::ClientEntityList->GetClientEntityFromHandle(pWeapon->m_hOwnerEntity()) : nullptr;
		auto pLocal = H::Entities.GetLocal();
		if (pWeapon && pOwner == pLocal)
		{
			// credits: KGB

			int nOldTeam = pWeapon->m_iTeamNum();
			pWeapon->m_iTeamNum() = TF_TEAM_COUNT;
			auto ret = CALL_ORIGINAL(rcx, iIndex);
			pWeapon->m_iTeamNum() = nOldTeam;

			switch (FNV1A::Hash32(ret))
			{
			case FNV1A::Hash32Const("Weapon_FlameThrower.Fire"): return "MVM.GiantPyro_FlameStart";
			case FNV1A::Hash32Const("Weapon_FlameThrower.FireLoop"): return "MVM.GiantPyro_FlameLoop";
			case FNV1A::Hash32Const("Weapon_GrenadeLauncher.Single"): return "MVM.GiantDemoman_Grenadeshoot";
			}

			return ret;
		}
	}

	return CALL_ORIGINAL(rcx, iIndex);
}