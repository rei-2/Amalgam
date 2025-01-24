#pragma once
#include "../../../Utils/Feature/Feature.h"
#include "../../Definitions/Classes.h"
#include <unordered_map>

enum struct EGroupType
{
	PLAYERS_ALL, PLAYERS_ENEMIES, PLAYERS_TEAMMATES,
	BUILDINGS_ALL, BUILDINGS_ENEMIES, BUILDINGS_TEAMMATES,
	PICKUPS_HEALTH, PICKUPS_AMMO, PICKUPS_MONEY, PICKUPS_POWERUP, PICKUPS_SPELLBOOK,
	WORLD_PROJECTILES, WORLD_OBJECTIVE, WORLD_NPC, WORLD_BOMBS, WORLD_GARGOYLE,
	MISC_LOCAL_STICKIES, MISC_LOCAL_FLARES, MISC_DOTS
};

struct DormantData
{
	Vec3 Location;
	float LastUpdate = 0.f;
};

class CEntities
{
	CTFPlayer* m_pLocal = nullptr;
	CTFWeaponBase* m_pLocalWeapon = nullptr;
	CTFPlayerResource* m_pPlayerResource = nullptr;

	std::unordered_map<EGroupType, std::vector<CBaseEntity*>> m_mGroups = {};

	std::unordered_map<int, float> m_mSimTimes, m_mOldSimTimes, m_mDeltaTimes;
	std::unordered_map<int, int> m_mChokes, m_mSetTicks;
	std::unordered_map<int, std::pair<bool, matrix3x4[MAXSTUDIOBONES]>> m_mBones;
	std::unordered_map<int, Vec3> m_mEyeAngles, m_mPingAngles;
	std::unordered_map<int, bool> m_mLagCompensation;
	std::unordered_map<int, DormantData> m_mDormancy;
	std::unordered_map<int, Vec3> m_mAvgVelocities;
	std::unordered_map<int, uint32_t> m_mModels;

	std::unordered_map<int, bool> m_mIFriends;
	std::unordered_map<uint32_t, bool> m_mUFriends;
	std::unordered_map<int, bool> m_mIParty;
	std::unordered_map<uint32_t, bool> m_mUParty;
	std::unordered_map<int, int> m_mIPriorities;
	std::unordered_map<uint32_t, int> m_mUPriorities;

	bool m_bSettingUpBones = false;

public:
	void Store();
	void Clear(bool bShutdown = false);
	void ManualNetwork(const StartSoundParams_t& params);

	bool IsHealth(uint32_t uHash);
	bool IsAmmo(uint32_t uHash);
	bool IsPowerup(uint32_t uHash);
	bool IsSpellbook(uint32_t uHash);

	CTFPlayer* GetLocal();
	CTFWeaponBase* GetWeapon();
	CTFPlayerResource* GetPR();

	const std::vector<CBaseEntity*>& GetGroup(const EGroupType& Group);

	float GetSimTime(CBaseEntity* pEntity);
	float GetOldSimTime(CBaseEntity* pEntity);
	float GetDeltaTime(int iIndex);
	int GetChoke(int iIndex);
	matrix3x4* GetBones(int iIndex);
	Vec3 GetEyeAngles(int iIndex);
	Vec3 GetPingAngles(int iIndex);
	bool GetLagCompensation(int iIndex);
	void SetLagCompensation(int iIndex, bool bLagComp);
	bool GetDormancy(int iIndex);
	Vec3* GetAvgVelocity(int iIndex);
	void SetAvgVelocity(int iIndex, Vec3 vAvgVelocity);
	uint32_t GetModel(int iIndex);

	bool IsFriend(int iIndex);
	bool IsFriend(uint32_t friendsID);
	bool InParty(int iIndex);
	bool InParty(uint32_t friendsID);
	int GetPriority(int iIndex);
	int GetPriority(uint32_t friendsID);

	bool IsSettingUpBones();
};

ADD_FEATURE_CUSTOM(CEntities, Entities, H);