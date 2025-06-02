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

	CHandle<CTFWeaponBase>(&m_hMyWeapons())[MAX_WEAPONS];
	CTFWeaponBase* GetWeaponFromSlot(int nSlot);
};