#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFWeaponBase_GetShootSound, "client.dll", "40 55 56 41 56 48 83 EC ? 80 B9", 0x0);

MAKE_HOOK(CTFWeaponBase_GetShootSound, S::CTFWeaponBase_GetShootSound(), const char*, __fastcall,
	void* ecx, int iIndex)
{
	if (Vars::Misc::Sound::GiantWeaponSounds.Value)
	{
		auto pWeapon = reinterpret_cast<CBaseCombatWeapon*>(ecx);
		auto pOwner = pWeapon ? I::ClientEntityList->GetClientEntityFromHandle(pWeapon->m_hOwnerEntity()) : nullptr;
		auto pLocal = H::Entities.GetLocal();
		if (pWeapon && pOwner == pLocal)
		{
			int nOldTeam = pWeapon->m_iTeamNum();
			pWeapon->m_iTeamNum() = 4;
			auto ret = CALL_ORIGINAL(ecx, iIndex);
			pWeapon->m_iTeamNum() = nOldTeam;

			// credits: KGB

			if (FNV1A::Hash(ret) == FNV1A::HashConst("Weapon_FlameThrower.Fire"))
				return "MVM.GiantPyro_FlameStart";

			if (FNV1A::Hash(ret) == FNV1A::HashConst("Weapon_FlameThrower.FireLoop"))
				return "MVM.GiantPyro_FlameLoop";

			if (FNV1A::Hash(ret) == FNV1A::HashConst("Weapon_GrenadeLauncher.Single"))
				return "MVM.GiantDemoman_Grenadeshoot";

			return ret;
		}
	}

	return CALL_ORIGINAL(ecx, iIndex);
}