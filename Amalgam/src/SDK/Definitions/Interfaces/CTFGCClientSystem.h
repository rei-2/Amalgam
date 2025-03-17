#include "Interface.h"
#include "../Steam/SteamClientPublic.h"
#include "../../../Utils/Memory/Memory.h"

MAKE_SIGNATURE(CGCClientSharedObjectCache_FindTypeCache, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F B7 59 ? BE", 0x0);

MAKE_SIGNATURE(CTFGCClientSystem_PingThink, "client.dll", "48 89 4C 24 ? 55 41 54 41 55 48 8D AC 24 ? ? ? ? 48 81 EC", 0x0);
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
	inline CGCClientSharedObjectTypeCache* FindTypeCache(int nClassID)
	{
		return S::CGCClientSharedObjectCache_FindTypeCache.Call<CGCClientSharedObjectTypeCache*>(this, nClassID);
	}
};

class ConstTFLobbyPlayer
{
	void* pad0;
	void* pad1;

public:
	inline void* Proto()
	{
		auto ConstTFLobbyPlayer_Proto = reinterpret_cast<void*(*)(void*)>(U::Memory.GetVFunc(this, 0));
		return ConstTFLobbyPlayer_Proto(this);
	}
};

class CTFLobbyShared
{
public:
	inline int64 GetNumMembers()
	{
		auto CTFLobbyShared_GetNumMembers = reinterpret_cast<int(*)(void*)>(U::Memory.GetVFunc(this, 2));
		return CTFLobbyShared_GetNumMembers(this);
	}

	inline CSteamID* GetMember(CSteamID* pSteamID, int i)
	{
		auto CTFLobbyShared_GetMember = reinterpret_cast<CSteamID*(*)(void*, CSteamID*, int)>(U::Memory.GetVFunc(this, 3));
		return CTFLobbyShared_GetMember(this, pSteamID, i);
	}

	inline int64 GetMemberIndexBySteamID(CSteamID& cSteamID)
	{
		auto CTFLobbyShared_GetMemberIndexBySteamID = reinterpret_cast<int(*)(void*, CSteamID&)>(U::Memory.GetVFunc(this, 4));
		return CTFLobbyShared_GetMemberIndexBySteamID(this, cSteamID);
	}

	inline ConstTFLobbyPlayer* GetMemberDetails(ConstTFLobbyPlayer* pDetails, int i)
	{
		auto CTFLobbyShared_GetMemberDetails = reinterpret_cast<ConstTFLobbyPlayer*(*)(void*, ConstTFLobbyPlayer*, int)>(U::Memory.GetVFunc(this, 13));
		return CTFLobbyShared_GetMemberDetails(this, pDetails, i);
	}
};

class CTFParty
{
public:
	inline int64 GetNumMembers()
	{
		auto pParty = reinterpret_cast<void*>(uintptr_t(this) + 184);
		auto CTFParty_GetNumMembers = reinterpret_cast<int64(*)(void*)>(U::Memory.GetVFunc(pParty, 2));
		return CTFParty_GetNumMembers(pParty);
	}

	inline CSteamID* GetMember(CSteamID* pSteamID, int i)
	{
		auto pParty = reinterpret_cast<void*>(uintptr_t(this) + 184);
		auto CTFParty_GetMember = reinterpret_cast<CSteamID*(*)(void*, CSteamID*, int)>(U::Memory.GetVFunc(pParty, 3));
		return CTFParty_GetMember(pParty, pSteamID, i);
	}

	inline void SpewDebug()
	{
		S::CTFParty_SpewDebug.Call<void>(this);
	}
};

class CTFGCClientSystem
{
public:
	inline CGCClientSharedObjectCache* m_pSOCache()
	{
		return *reinterpret_cast<CGCClientSharedObjectCache**>(uintptr_t(this) + 1072);
	}

	inline void PingThink()
	{
		S::CTFGCClientSystem_PingThink.Call<void>(this);
	}

	inline CTFParty* GetParty()
	{
		return S::CTFGCClientSystem_GetParty.Call<CTFParty*>(this);
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