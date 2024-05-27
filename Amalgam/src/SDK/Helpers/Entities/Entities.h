#pragma once
#include "../../../Utils/Feature/Feature.h"
#include "../../Definitions/Classes.h"
#include <unordered_map>

enum struct EGroupType
{
	PLAYERS_ALL, PLAYERS_ENEMIES, PLAYERS_TEAMMATES,
	BUILDINGS_ALL, BUILDINGS_ENEMIES, BUILDINGS_TEAMMATES,
	WORLD_HEALTH, WORLD_AMMO, WORLD_PROJECTILES, WORLD_NPC, WORLD_BOMBS, WORLD_MONEY, WORLD_SPELLBOOK, WORLD_GARGOYLE,
	MISC_LOCAL_STICKIES, MISC_LOCAL_FLARES, MISC_DOTS
};

class CEntities
{
	CTFPlayer* m_pLocal = nullptr;
	CTFWeaponBase* m_pLocalWeapon = nullptr;
	CTFPlayerResource* m_pPlayerResource = nullptr;
	CTFPlayer* m_pObservedTarget = nullptr;

	std::unordered_map<EGroupType, std::vector<CBaseEntity*>> m_mGroups = {};
	std::unordered_map<uint32_t, int> m_mIDIndex = {};
	std::unordered_map<int, bool> m_mFriends;
	std::unordered_map<uint32_t, int> m_mUPriorities;
	std::unordered_map<int, int> m_mIPriorities;

public:
	void Fill();
	void Clear();

	bool IsHealth(CBaseEntity* pEntity);
	bool IsAmmo(CBaseEntity* pEntity);
	bool IsSpellbook(CBaseEntity* pEntity);

	CTFPlayer* GetLocal() { return m_pLocal; }
	CTFWeaponBase* GetWeapon() { return m_pLocalWeapon; }
	CTFPlayerResource* GetPR() { return m_pPlayerResource; }
	CTFPlayer* GetObservedTarget() { return m_pObservedTarget; }

	const std::vector<CBaseEntity*>& GetGroup(const EGroupType& Group) { return m_mGroups[Group]; }
	bool IsFriend(int iIndex) { return m_mFriends[iIndex]; }
	bool IsFriend(uint32_t friendsID) { return m_mFriends[m_mIDIndex[friendsID]]; }
	int GetPriority(uint32_t friendsID) { return m_mUPriorities[friendsID]; }
	int GetPriority(int iIndex) { return m_mIPriorities[iIndex]; }
};

ADD_FEATURE_CUSTOM(CEntities, Entities, H);