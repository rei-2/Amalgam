#pragma once
#include "../../../SDK/SDK.h"

Enum(Targets, Players = 1 << 0, Buildings = 1 << 1, Projectiles = 1 << 2, Ragdolls = 1 << 3, Objective = 1 << 4, NPCs = 1 << 5, Health = 1 << 6, Ammo = 1 << 7, Money = 1 << 8, Powerups = 1 << 9, Bombs = 1 << 10, Spellbook = 1 << 11, Gargoyle = 1 << 12)
Enum(Conditions, Relative = 1 << 0, Enemy = 1 << 1, Team = 1 << 2, BLU = 1 << 3, RED = 1 << 4, Local = 1 << 5, Friends = 1 << 6, Party = 1 << 7, Priority = 1 << 8, Target = 1 << 9, Scout = 1 << 10, Soldier = 1 << 11, Pyro = 1 << 12, Demoman = 1 << 13, Heavy = 1 << 14, Engineer = 1 << 15, Medic = 1 << 16, Sniper = 1 << 17, Spy = 1 << 18)
Enum(ESP, Name = 1 << 0, Box = 1 << 1, Distance = 1 << 2, Bones = 1 << 3, HealthBar = 1 << 4, HealthText = 1 << 5, UberBar = 1 << 6, UberText = 1 << 7, ClassIcon = 1 << 8, ClassText = 1 << 9, WeaponIcon = 1 << 10, WeaponText = 1 << 11, Priority = 1 << 12, Labels = 1 << 13, Buffs = 1 << 14, Debuffs = 1 << 15, Misc = 1 << 16, Lag = 1 << 17, Ping = 1 << 18, KDR = 1 << 19, Owner = 1 << 20, Flags = 1 << 21, Level = 1 << 22, IntelReturnTime = 1 << 23)
Enum(Backtrack, Last = 1 << 0, First = 1 << 1, Always = 1 << 2, IgnoreTeam = 1 << 3)

struct Group_t
{
	std::string m_sName = "";
	int m_iTargets = 0b0; // ragdolls don't draw esp
	int m_iConditions = 0b11001; // red/blu only apply if not relative, classes only apply if at least 1 is selected

	int m_iESP = 0b0;

	bool m_bOutOfFOVArrows = false;
	int m_iOutOfFOVArrowsOffset = 100;
	float m_flOutOfFOVArrowsMaxDistance = 1000.f;

	int m_iActiveAlpha = 255;
	int m_iDormantAlpha = 100;
	float m_flDormantDuration = 1.f;

	bool m_bChams = false;
	Chams_t m_tChams = {};

	bool m_bGlow = false;
	Glow_t m_tGlow = {};

	// keep backtrack separate?
	bool m_bBacktrack = false;
	int m_iBacktrackDraw = 0b0;
	Chams_t m_tBacktrackChams = {};
	bool m_bBacktrackIgnoreZ = false;
	Glow_t m_tBacktrackGlow = {};
};

class CGroups
{
private:
	bool ShouldTarget(Group_t& tGroup, CBaseEntity* pEntity, CTFPlayer* pLocal);
	bool ShouldTargetOwner(bool bType, Group_t& tGroup, CBaseEntity* pOwner, CBaseEntity* pEntity, CTFPlayer* pLocal);
	bool ShouldTargetTeam(bool bType, Group_t& tGroup, CBaseEntity* pEntity, CTFPlayer* pLocal);

public:
	// GetESP
	// GetChams
	// GetGlow
	// GetBacktrackChams?
	// GetBacktrackGlow?

	std::vector<Group_t> m_vGroups = {}; // loop through this in reverse so back groups have higher priority
};

ADD_FEATURE(CGroups, Groups)