#pragma once
#include "../../SDK/SDK.h"

Enum(Bind, Key, Class, WeaponType, ItemSlot)
namespace BindEnum
{
	Enum(Key, Hold, Toggle, DoubleClick)
	Enum(Class, Scout, Soldier, Pyro, Demoman, Heavy, Engineer, Medic, Sniper, Spy)
	Enum(WeaponType, Hitscan, Projectile, Melee)
	//Enum(ItemType, First, Second, Third, Fourth, Fifth, Sixth, Seventh, Eighth, Ninth)
}

struct Bind_t
{
	std::string m_sName = "";
	int m_iType = 0;
	int m_iInfo = 0;
	int m_iKey = 0;
	bool m_bNot = false;

	bool m_bActive = false;
	bool m_bVisible = true;

	int m_iParent = -1;
};

class CBinds
{
public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	bool GetBind(int iID, Bind_t* pBind);

	bool HasChildren(int iBind);
	int GetParent(int iBind);
	void AddBind(int iBind, Bind_t& tCond);
	void RemoveBind(int iBind, bool bForce = true);

	std::vector<Bind_t> m_vBinds = {};
	std::unordered_map<int, KeyStorage> m_mKeyStorage;
};

ADD_FEATURE(CBinds, Binds)