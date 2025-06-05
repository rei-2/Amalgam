#pragma once
#include "CEconEntity.h"

class CBaseCombatWeapon : public CEconEntity
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

	NETVAR_OFF(m_bInReload, bool, "CBaseCombatWeapon", "m_flTimeWeaponIdle", 4);
	NETVAR_OFF(m_bRemoveable, bool, "CBaseCombatWeapon", "m_iState", -12);

	VIRTUAL(CanBeSelected, bool, 233, this);
	VIRTUAL(GetName, const char*, 333, this);
};