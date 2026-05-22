#pragma once
#include "../../../Utils/Macros/Macros.h"
#include "../../Definitions/Classes.h"
#include "../../Vars.h"

Enum(Entity,
	PlayerAll, PlayerEnemy, PlayerTeam,
	BuildingAll, BuildingEnemy, BuildingTeam,
	PickupHealth, PickupAmmo, PickupMoney, PickupPowerup, PickupSpellbook, PickupGargoyle,
	WorldProjectile, WorldObjective, WorldNPC, WorldBomb,
	LocalStickies, LocalFlares, SniperDots,
	Invalid, GroupsMax
)

struct DormantData
{
	Vec3 m_vLocation;
	float m_flLastUpdate = 0.f;
};

struct VelFixRecord
{
	Vec3 m_vecOrigin;
	float m_flSimulationTime;
};

class CEntities
{
private:
	bool ManageDormancy(CBaseEntity* pEntity);

	CTFPlayer* m_pLocal = nullptr;
	CTFWeaponBase* m_pLocalWeapon = nullptr;
	CTFPlayerResource* m_pPlayerResource = nullptr;

	std::array<std::vector<CBaseEntity*>, EntityEnum::GroupsMax> m_aGroups = {};

	std::array<float, MAX_PLAYERS> m_aSimTimes = {}, m_aDeltaTimes = {}, m_aLagTimes = {};
	std::array<int, MAX_PLAYERS> m_aChokes = {}, m_aSetTicks = {};
	std::array<Vec3, MAX_PLAYERS> m_aOldAngles = {}, m_aEyeAngles = {};
	std::array<bool, MAX_PLAYERS> m_aLagCompensation = {};
	std::array<Vec3, MAX_PLAYERS> m_aAvgVelocities = {};
	std::array<std::deque<VelFixRecord>, MAX_PLAYERS> m_aOrigins = {};
	std::array<uint32_t, MAX_EDICTS> m_aModels = {};

	std::unordered_map<int, int> m_mIPriorities = {};
	std::unordered_map<uint32_t, int> m_mUPriorities = {};
	std::unordered_map<int, bool> m_mIFriends = {};
	std::unordered_map<uint32_t, bool> m_mUFriends = {};
	std::unordered_map<int, int> m_mIParty = {};
	std::unordered_map<uint32_t, int> m_mUParty = {};
	std::unordered_map<int, bool> m_mIF2P = {};
	std::unordered_map<uint32_t, bool> m_mUF2P = {};
	std::unordered_map<int, int> m_mILevels = {};
	std::unordered_map<uint32_t, int> m_mULevels = {};
	uint32_t m_uAccountID;
	int m_iPartyCount = 0;

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
	CTFPlayerResource* GetResource();

	const std::vector<CBaseEntity*>& GetGroup(uint8_t iGroup);

	float GetDeltaTime(uint16_t iIndex);
	float GetLagTime(uint16_t iIndex);
	int GetChoke(uint16_t iIndex);
	Vec3 GetEyeAngles(uint16_t iIndex);
	Vec3 GetDeltaAngles(uint16_t iIndex);
	bool GetLagCompensation(uint16_t iIndex);
	void SetLagCompensation(uint16_t iIndex, bool bLagComp);
	Vec3* GetAvgVelocity(uint16_t iIndex);
	void SetAvgVelocity(uint16_t iIndex, Vec3 vAvgVelocity);
	std::deque<VelFixRecord>* GetOrigins(uint16_t iIndex);
	uint32_t GetModel(unsigned short iIndex);
	DormantData* GetDormancy(unsigned short iIndex);

	int GetPriority(int iIndex);
	int GetPriority(uint32_t uAccountID);
	bool IsFriend(int iIndex);
	bool IsFriend(uint32_t uAccountID);
	bool InParty(int iIndex);
	bool InParty(uint32_t uAccountID);
	bool IsF2P(int iIndex);
	bool IsF2P(uint32_t uAccountID);
	int GetLevel(int iIndex);
	int GetLevel(uint32_t uAccountID);
	int GetParty(int iIndex);
	int GetParty(uint32_t uAccountID);
	int GetPartyCount();
};

ADD_FEATURE_CUSTOM(CEntities, Entities, H);