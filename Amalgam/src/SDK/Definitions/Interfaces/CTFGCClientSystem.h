#pragma once
#include "Interface.h"
#include "../Steam/SteamClientPublic.h"

MAKE_SIGNATURE(CGCClientSharedObjectCache_FindTypeCache, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B7 59 ? BE", 0x0);

MAKE_SIGNATURE(CTFGCClientSystem_PingThink, "client.dll", "40 55 41 54 41 55 48 8D AC 24", 0x0);
MAKE_SIGNATURE(CTFGCClientSystem_UpdateAssignedLobby, "client.dll", "40 55 53 41 54 41 56 41 57 48 8B EC", 0x0);
MAKE_SIGNATURE(CTFGCClientSystem_GetParty, "client.dll", "48 83 EC ? 48 8B 89 ? ? ? ? 48 85 C9 74 ? BA ? ? ? ? E8 ? ? ? ? 48 85 C0 74 ? 8B 48 ? 85 C9 74 ? 48 8B 40 ? FF C9", 0x0);

MAKE_SIGNATURE(CTFParty_SpewDebug, "client.dll", "4C 8B DC 41 56 48 81 EC ? ? ? ? 8B 05", 0x0);

class CGCClientSharedObjectTypeCache
{
public:
	inline int GetCacheCount()
	{
		return *reinterpret_cast<int*>(uintptr_t(this) + 40);
	}
};

class CGCClientSharedObjectCache
{
public:
	SIGNATURE_ARGS(FindTypeCache, CGCClientSharedObjectTypeCache*, CGCClientSharedObjectCache, (int nClassID), this, nClassID);
};

struct CTFLobbyPlayerProto
{
	enum TF_GC_TEAM
	{
		TF_GC_TEAM_DEFENDERS = 0,
		TF_GC_TEAM_INVADERS = 1,
		TF_GC_TEAM_BROADCASTER = 2,
		TF_GC_TEAM_SPECTATOR = 3,
		TF_GC_TEAM_PLAYER_POOL = 4,
		TF_GC_TEAM_NOTEAM = 5
	};

	enum ConnectState
	{
		INVALID = 0,
		RESERVATION_PENDING = 1,
		RESERVED = 2,
		CONNECTED = 3,
		CONNECTED_AD_HOC = 4,
		DISCONNECTED = 5
	};

	enum Type {
		INVALID_PLAYER = 0,
		MATCH_PLAYER = 1,
		STANDBY_PLAYER = 2,
		OBSERVING_PLAYER = 3
	};

	byte pad0[24];
	uint64 id;
	TF_GC_TEAM team;
	ConnectState connect_state;
	const char* name;
	uint64 original_party_id;
	uint32 badge_level;
	uint32 last_connect_time;
	Type type;
	bool squad_surplus;
	bool chat_suspension;
	double normalized_rating;
	double normalized_uncertainty;
	uint32 rank;
};

class ConstTFLobbyPlayer
{
	void* pad0;
	void* pad1;

public:
	VIRTUAL(Proto, CTFLobbyPlayerProto*, 0, this);
};

class CTFLobbyShared
{
public:
	VIRTUAL(GetNumMembers, int, 2, this);
	VIRTUAL_ARGS(GetMember, CSteamID*, 3, (CSteamID* pSteamID, int i), this, pSteamID, i);
	VIRTUAL_ARGS(GetMemberIndexBySteamID, int, 4, (CSteamID& pSteamID), this, std::ref(pSteamID));
	VIRTUAL_ARGS(GetMemberDetails, ConstTFLobbyPlayer*, 13, (ConstTFLobbyPlayer* pDetails, int i), this, pDetails, i);
};

class CTFParty
{
public:
	VIRTUAL(GetNumMembers, int64, 2, uintptr_t(this) + 184);
	VIRTUAL_ARGS(GetMember, CSteamID*, 3, (CSteamID* pSteamID, int i), uintptr_t(this) + 184, pSteamID, i);

	SIGNATURE(SpewDebug, void, CTFParty, this);
};

class CTFGCClientSystem
{
public:
	SIGNATURE(PingThink, void, CTFGCClientSystem, this);
	SIGNATURE(GetParty, CTFParty*, CTFGCClientSystem, this);

	inline CGCClientSharedObjectCache* m_pSOCache()
	{
		return *reinterpret_cast<CGCClientSharedObjectCache**>(uintptr_t(this) + 1072);
	}

	inline CTFLobbyShared* GetLobby()
	{
		auto pSOCache = m_pSOCache();
		if (!pSOCache)
			return nullptr;

		auto pTypeCache = pSOCache->FindTypeCache(2004);
		if (!pTypeCache)
			return nullptr;

		int iCacheCount = pTypeCache->GetCacheCount();
		if (!iCacheCount)
			return nullptr;

		auto pLobby = *reinterpret_cast<CTFLobbyShared**>(*reinterpret_cast<uintptr_t*>(uintptr_t(pTypeCache) + 8) + 8 * uintptr_t(iCacheCount - 1));
		if (!pLobby)
			return nullptr;

		return reinterpret_cast<CTFLobbyShared*>(uintptr_t(pLobby) - 8); // i assume from the dynamic_cast?
	}

	inline void SetPendingPingRefresh(bool bValue)
	{
		*reinterpret_cast<bool*>(uintptr_t(this) + 1116) = bValue;
	}

	inline void SetNonPremiumAccount(bool bValue)
	{
		*reinterpret_cast<bool*>(uintptr_t(this) + 1888) = bValue;
	}
};

MAKE_INTERFACE_SIGNATURE(CTFGCClientSystem, TFGCClientSystem, "client.dll", "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8B 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24", 0x0, 0);