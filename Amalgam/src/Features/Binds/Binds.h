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
Enum(BindVisibility, Always, WhileActive, Hidden)

struct Bind_t
{
	std::string m_sName = "";
	int m_iType = 0;
	int m_iInfo = 0;
	int m_iKey = 0;

	bool m_bEnabled = true;
	int m_iVisibility = BindVisibilityEnum::Always;
	bool m_bNot = false;
	bool m_bActive = false;
	KeyStorage m_tKeyStorage = {};

	int m_iParent = DEFAULT_BIND;

	std::vector<BaseVar*> m_vVars = {};
};

class CBinds
{
public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	bool GetBind(int iID, Bind_t* pBind);

	void AddBind(int iBind, Bind_t& tCond);
	void RemoveBind(int iBind, bool bForce = true);
	int GetParent(int iBind);
	bool HasChildren(int iBind);
	bool WillBeEnabled(int iBind);

	std::vector<Bind_t> m_vBinds = {};
};

ADD_FEATURE(CBinds, Binds)