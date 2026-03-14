#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayer_GetMinFOV, "client.dll", "F3 0F 10 05 ? ? ? ? C3 CC CC CC CC CC CC CC 48 8B 81 ? ? ? ? 8B D2", 0x0);
MAKE_SIGNATURE(CBasePlayer_GetFOV, "client.dll", "40 53 48 83 EC ? 48 8B D9 0F 29 74 24 ? 48 8B 0D ? ? ? ? 48 8B 01", 0x0);

MAKE_HOOK(CTFPlayer_GetMinFOV, S::CTFPlayer_GetMinFOV(), float,
	/*void* rcx*/)
{
	DEBUG_RETURN(CTFPlayer_GetMinFOV, /*rcx*/);

	return 0.f;
}

MAKE_HOOK(CBasePlayer_GetFOV, S::CBasePlayer_GetFOV(), float,
	void* rcx)
{
	DEBUG_RETURN(CTFPlayer_GetMinFOV, rcx);

	auto pPlayer = reinterpret_cast<CBasePlayer*>(rcx);
	if (pPlayer->entindex() != I::EngineClient->GetLocalPlayer())
		return CALL_ORIGINAL(rcx);

	int iOriginalFOV = pPlayer->m_iFOV();
	pPlayer->m_iFOV() = ceilf(G::FOV);
	float flReturn = CALL_ORIGINAL(rcx);
	pPlayer->m_iFOV() = iOriginalFOV;
	return ceilf(flReturn);
}