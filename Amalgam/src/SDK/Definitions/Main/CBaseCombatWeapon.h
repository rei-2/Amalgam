#pragma once
#include "CBaseProjectile.h"
#include "../../../Utils/Signatures/Signatures.h"

MAKE_SIGNATURE(CBaseCombatWeapon_HasAmmo, "client.dll", "40 53 48 83 EC ? 83 B9 ? ? ? ? ? 48 8B D9 75 ? 83 B9 ? ? ? ? ? 74 ? 48 8B 01", 0x0);

class CBaseCombatWeapon : public CBaseAnimating
{
public:
	NETVAR(m_iClip1, int, "CBaseCombatWeapon", "m_iClip1");
	NETVAR(m_iClip2, int, "CBaseCombatWeapon", "m_iClip2");
	NETVAR(m_iPrimaryAmmoType, int, "CBaseCombatWeapon", "m_iPrimaryAmmoType");
	NETVAR(m_iSecondaryAmmoType, int, "CBaseCombatWeapon", "m_iSecondaryAmmoType");
	NETVAR(m_nViewModelIndex, int, "CBaseCombatWeapon", "m_nViewModelIndex");
	NETVAR(m_bFlipViewModel, bool, "CBaseCombatWeapon", "m_bFlipViewModel");
	NETVAR(m_flNextPrimaryAttack, float, "CBaseCombatWeapon", "m_flNextPrimaryAttack");
	NETVAR(m_flNextSecondaryAttack, float, "CBaseCombatWeapon", "m_flNextSecondaryAttack");
	NETVAR(m_nNextThinkTick, int, "CBaseCombatWeapon", "m_nNextThinkTick");
	NETVAR(m_flTimeWeaponIdle, float, "CBaseCombatWeapon", "m_flTimeWeaponIdle");
	NETVAR(m_iViewModelIndex, int, "CBaseCombatWeapon", "m_iViewModelIndex");
	NETVAR(m_iWorldModelIndex, int, "CBaseCombatWeapon", "m_iWorldModelIndex");
	NETVAR(m_iState, int, "CBaseCombatWeapon", "m_iState");
	NETVAR(m_hOwner, EHANDLE, "CBaseCombatWeapon", "m_hOwner");

	bool HasAmmo()
	{
		return S::CBaseCombatWeapon_HasAmmo.As<bool(__thiscall*)(void*)>()(this);
	}
};