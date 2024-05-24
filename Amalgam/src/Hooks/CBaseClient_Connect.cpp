#include "../SDK/SDK.h"

#include "../Features/Visuals/Materials/Materials.h"

MAKE_SIGNATURE(CBaseClient_Connect, "engine.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 49 8B E9 48 8D 0D", 0x0);

MAKE_HOOK(CBaseClient_Connect, S::CBaseClient_Connect(), void, __fastcall,
	void* ecx, const char* szName, int nUserID, INetChannel* pNetChannel, bool bFakePlayer, int clientChallenge)
{
	F::Materials.ReloadMaterials();

	CALL_ORIGINAL(ecx, szName, nUserID, pNetChannel, bFakePlayer, clientChallenge);
}