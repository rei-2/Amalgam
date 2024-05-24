#pragma once
#include "Interface.h"

MAKE_SIGNATURE(CTFPartyClient_LoadSavedCasualCriteria, "client.dll", "48 83 79 ? ? C6 81 ? ? ? ? ? 74 ? 80 79 ? ? 74 ? C6 81 ? ? ? ? ? 48 8B 15", 0x0);
MAKE_SIGNATURE(CTFPartyClient_BInQueueForMatchGroup, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B F9 8B DA 8B CA E8 ? ? ? ? 84 C0", 0x0);
MAKE_SIGNATURE(CTFPartyClient_RequestQueueForMatch, "client.dll", "40 55 56 48 81 EC ? ? ? ? 48 63 F2", 0x0);

class CTFPartyClient
{
public:
	void LoadSavedCasualCriteria()
	{
		return S::CTFPartyClient_LoadSavedCasualCriteria.As<void(__fastcall*)(void*)>()(this);
	}

	bool BInQueueForMatchGroup(int eMatchGroup)
	{
		return S::CTFPartyClient_BInQueueForMatchGroup.As<bool(__fastcall*)(void*, int)>()(this, eMatchGroup);
	}

	void RequestQueueForMatch(int eMatchGroup)
	{
		return S::CTFPartyClient_RequestQueueForMatch.As<void(__fastcall*)(void*, int)>()(this, eMatchGroup);
	}
};

MAKE_INTERFACE_NULL(CTFPartyClient, TFPartyClient);