#include "Interface.h"
#include "../Steam/SteamClientPublic.h"

MAKE_SIGNATURE(CTFGCClientSystem_PingThink, "client.dll", "48 89 4C 24 ? 55 41 54 41 55 48 8D AC 24 ? ? ? ? 48 81 EC", 0x0);
MAKE_SIGNATURE(CTFGCClientSystem_UpdateAssignedLobby, "client.dll", "40 55 53 41 54 41 56 41 57 48 8B EC", 0x0);
MAKE_SIGNATURE(CTFGCClientSystem_GetParty, "client.dll", "48 83 EC ? 48 8B 89 ? ? ? ? 48 85 C9 74 ? BA ? ? ? ? E8 ? ? ? ? 48 85 C0 74 ? 8B 48 ? 85 C9 74 ? 48 8B 40 ? FF C9", 0x0);

MAKE_SIGNATURE(CTFParty_GetNumMembers, "client.dll", "8B 41 ? C3 CC CC CC CC CC CC CC CC CC CC CC CC 8B 41 ? C3 CC CC CC CC CC CC CC CC CC CC CC CC 48 8B C1", 0x0);
MAKE_SIGNATURE(CTFParty_GetMember, "client.dll", "48 8B C1 33 C9 45 85 C0 78 ? 44 3B 40 ? 7D ? 48 8B 80", 0x0);
MAKE_SIGNATURE(CTFParty_SpewDebug, "client.dll", "4C 8B DC 41 56 48 81 EC ? ? ? ? 8B 05", 0x0);

class CTFParty
{
public:
	inline int64 GetNumMembers()
	{
		return S::CTFParty_GetNumMembers.Call<int64>(uintptr_t(this) + 184);
	}

	inline CSteamID* GetMember(CSteamID* pSteamID, int i)
	{
		return S::CTFParty_GetMember.Call<CSteamID*>(uintptr_t(this) + 184, pSteamID, i);
	}

	inline void SpewDebug()
	{
		S::CTFParty_SpewDebug.Call<void>(this);
	}
};

class CTFGCClientSystem
{
public:
	inline void PingThink()
	{
		S::CTFGCClientSystem_PingThink.Call<void>(this);
	}

	inline CTFParty* GetParty()
	{
		return S::CTFGCClientSystem_GetParty.Call<CTFParty*>(this);
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