#pragma once
#include "../../../SDK/SDK.h"

Enum(Targets, Players = 1 << 0, Buildings = 1 << 1, Projectiles = 1 << 2, Ragdolls = 1 << 3, Objective = 1 << 4, NPCs = 1 << 5, Health = 1 << 6, Ammo = 1 << 7, Money = 1 << 8, Powerups = 1 << 9, Spellbook = 1 << 10, Bombs = 1 << 11, Gargoyle = 1 << 12, FakeAngle = 1 << 13, ViewmodelWeapon = 1 << 14, ViewmodelHands = 1 << 15, ESP = ~(Ragdolls | FakeAngle | ViewmodelWeapon | ViewmodelHands), Occluded = ~(FakeAngle | ViewmodelWeapon | ViewmodelHands))
Enum(Conditions, Enemy = 1 << 0, Team = 1 << 1, BLU = 1 << 2, RED = 1 << 3, Local = 1 << 4, Friends = 1 << 5, Party = 1 << 6, Priority = 1 << 7, Target = 1 << 8, Dormant = 1 << 9)
Enum(Player, Scout = 1 << 0, Soldier = 1 << 1, Pyro = 1 << 2, Demoman = 1 << 3, Heavy = 1 << 4, Engineer = 1 << 5, Medic = 1 << 6, Sniper = 1 << 7, Spy = 1 << 8, Invulnerable = 1 << 9, Crits = 1 << 10, Invisible = 1 << 11, Disguise = 1 << 12, Hurt = 1 << 13, Classes = Scout | Soldier | Pyro | Demoman | Heavy | Engineer | Medic | Sniper | Spy, Conds = Invulnerable | Crits | Invisible | Disguise | Hurt)
Enum(Building, Sentry = 1 << 0, Dispenser = 1 << 1, Teleporter = 1 << 2, Hurt = 1 << 3, Classes = Sentry | Dispenser | Teleporter, Conds = Hurt)
Enum(Projectile, Rocket = 1 << 0, Sticky = 1 << 1, Pipe = 1 << 2, Arrow = 1 << 3, Heal = 1 << 4, Flare = 1 << 5, Fire = 1 << 6, Repair = 1 << 7, Cleaver = 1 << 8, Milk = 1 << 9, Jarate = 1 << 10, Gas = 1 << 11, Bauble = 1 << 12, Baseball = 1 << 13, Energy = 1 << 14, ShortCircuit = 1 << 15, MeteorShower = 1 << 16, Lightning = 1 << 17, Fireball = 1 << 18, Bomb = 1 << 19, Bats = 1 << 20, Pumpkin = 1 << 21, Monoculus = 1 << 22, Skeleton = 1 << 23, Misc = 1 << 24, Crit = 1 << 25, Minicrit = 1 << 26, Classes = Rocket | Sticky | Pipe | Arrow | Heal | Flare | Fire | Repair | Cleaver | Milk | Jarate | Gas | Bauble | Baseball | Energy | ShortCircuit | MeteorShower | Lightning | Fireball | Bomb | Bats | Pumpkin | Monoculus | Skeleton | Misc, Conds = Crit | Minicrit)
Enum(ESP, Name = 1 << 0, Box = 1 << 1, Distance = 1 << 2, Bones = 1 << 3, HealthBar = 1 << 4, HealthText = 1 << 5, UberBar = 1 << 6, UberText = 1 << 7, ClassIcon = 1 << 8, ClassText = 1 << 9, WeaponIcon = 1 << 10, WeaponText = 1 << 11, Priority = 1 << 12, Labels = 1 << 13, Buffs = 1 << 14, Debuffs = 1 << 15, Flags = 1 << 16, LagCompensation = 1 << 17, Ping = 1 << 18, KDR = 1 << 19, Owner = 1 << 20, Level = 1 << 21, AmmoBars = 1 << 22, AmmoText = 1 << 23, IntelReturnTime = 1 << 24)
Enum(Backtrack, Enabled = 1 << 0, IgnoreZ = 1 << 1, Last = 1 << 2, First = 1 << 3, Always = 1 << 4)
Enum(Trajectory, Enabled = 1 << 0, IgnoreZ = 1 << 1, Predict = 1 << 2, Radius = 1 << 3, Trace = 1 << 4, Sphere = 1 << 5, Path = 1 << 6)
Enum(Sightlines, Enabled = 1 << 0, IgnoreZ = 1 << 1)

struct Group_t
{
	std::string m_sName = "";

	Color_t m_tColor = {};
	bool m_bTagsOverrideColor = true;

	int m_iTargets = 0b0;
	int m_iConditions = 0b0;
	int m_iPlayers = 0b0;
	int m_iBuildings = 0b0;
	int m_iProjectiles = 0b0;

	int m_iESP = 0b0;

	Chams_t m_tChams = {};

	Glow_t m_tGlow = {};

	bool m_bOffscreenArrows = false;
	int m_iOffscreenArrowsOffset = 100;
	float m_flOffscreenArrowsMaxDistance = 1000.f;

	bool m_bPickupTimer = false;

	int m_iBacktrack = 0b0;
	std::vector<std::pair<std::string, Color_t>> m_vBacktrackChams = {};
	Glow_t m_tBacktrackGlow = {};

	int m_iTrajectory = 0b0;

	int m_iSightlines = 0b10;
};

class CGroups
{
private:
	std::unordered_map<CBaseEntity*, Group_t*> m_mEntities = {};
	std::unordered_map<CBaseEntity*, Group_t*> m_mModels = {};

public:
	void Store(CTFPlayer* pLocal);

	bool GetGroup(CBaseEntity* pEntity, Group_t*& pGroup, bool bModels = true); // cached
	const std::unordered_map<CBaseEntity*, Group_t*>& GetGroup(bool bModels = true);

	bool GetGroup(CBaseEntity* pEntity, CTFPlayer* pLocal, Group_t*& pGroup, bool bModels = true);
	bool GetGroup(int iType, Group_t*& pGroup, CBaseEntity* pEntity = nullptr);
	bool GetGroup(int iType);

	Color_t GetColor(CBaseEntity* pEntity, Group_t* pGroup);
	bool GroupsActive();

	void Move(int i1, int i2);

	std::vector<Group_t> m_vGroups = {}; // loop through this in reverse so back groups have higher priority
};

ADD_FEATURE(CGroups, Groups);