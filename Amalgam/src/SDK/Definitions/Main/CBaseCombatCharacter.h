#pragma once
#include "CBaseFlex.h"

class CBaseCombatCharacter : public CBaseFlex
{
public:
	NETVAR(m_flNextAttack, float, "CBaseCombatCharacter", "m_flNextAttack");
	NETVAR(m_hActiveWeapon, EHANDLE, "CBaseCombatCharacter", "m_hActiveWeapon");
	NETVAR(m_hMyWeapons, EHANDLE, "CBaseCombatCharacter", "m_hMyWeapons");
	NETVAR(m_bGlowEnabled, bool, "CBaseCombatCharacter", "m_bGlowEnabled");

	__inline size_t* GetMyWeapons()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseCombatCharacter", "m_hMyWeapons");
		return reinterpret_cast<size_t*>(std::uintptr_t(this) + nOffset);
	}
};