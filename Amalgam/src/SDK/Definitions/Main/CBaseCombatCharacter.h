#pragma once
#include "CBaseFlex.h"

class CTFWeaponBase;

class CBaseCombatCharacter : public CBaseFlex
{
public:
	NETVAR(m_flNextAttack, float, "CBaseCombatCharacter", "m_flNextAttack");
	NETVAR(m_hActiveWeapon, EHANDLE, "CBaseCombatCharacter", "m_hActiveWeapon");
	//NETVAR(m_hMyWeapons, EHANDLE, "CBaseCombatCharacter", "m_hMyWeapons");
	NETVAR(m_bGlowEnabled, bool, "CBaseCombatCharacter", "m_bGlowEnabled");

	inline CHandle<CTFWeaponBase>(&m_hMyWeapons())[MAX_WEAPONS]
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseCombatCharacter", "m_hMyWeapons");
		return *reinterpret_cast<CHandle<CTFWeaponBase>(*)[MAX_WEAPONS]>(uintptr_t(this) + nOffset);
	}

	inline CTFWeaponBase* GetWeaponFromSlot(int nSlot)
	{
		if (nSlot < 0 || nSlot >= MAX_WEAPONS)
			return nullptr;
		return m_hMyWeapons()[nSlot].Get();
	}
};