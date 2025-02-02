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
	std::string Name = "";
	int Type = 0;
	int Info = 0;
	int Key = 0;
	bool Not = false;
	KeyStorage Storage = {};

	bool Active = false;
	bool Visible = true;

	int Parent = -1;
};

class CBinds
{
public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	bool GetBind(int iID, Bind_t* pBind);

	bool HasChildren(int iBind);
	int GetParent(int iBind);
	void AddBind(int iBind, Bind_t tCond);
	void RemoveBind(int iBind, bool bForce = true);

	std::vector<Bind_t> vBinds = {};
};

ADD_FEATURE(CBinds, Binds)