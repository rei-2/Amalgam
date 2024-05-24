#pragma once
#include "../../SDK/SDK.h"

struct Condition_t
{
	int Type = 0; // Key, Class, Weapon type
	int Info = 0; // Key: Hold, Toggle, Double click
				  // Class: Scout, Soldier, Pyro, Demoman, Heavy, Engineer, Medic, Sniper, Spy
				  // Weapon type: Hitscan, Projectile, Melee
	int Key = 0;
	bool Not = false;

	bool Active = false;
	bool Visible = true;
	KeyStorage Storage = {};

	std::string Parent = "";
};

class CConditions
{
public:
	void Run(CTFPlayer* pLocal, CTFWeaponBase* pWeapon);

	bool HasChildren(std::string sCondition);
	std::string GetParent(std::string sCondition);
	void AddCondition(std::string sCondition, Condition_t tCond);
	void RemoveCondition(std::string sCondition);

	std::unordered_map<std::string, Condition_t> mConditions = {};
	std::vector<std::string> vConditions = {}; // retain order
};

ADD_FEATURE(CConditions, Conditions)