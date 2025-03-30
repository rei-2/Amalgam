#include "../SDK/SDK.h"

static void POPID_ToString(SteamNetworkingPOPID popID, char* out)
{
	out[0] = static_cast<char>(popID >> 16);
	out[1] = static_cast<char>(popID >> 8);
	out[2] = static_cast<char>(popID);
	out[3] = static_cast<char>(popID >> 24);
	out[4] = 0;
}

unsigned int GetDatacenter(uint32_t uHash)
{
	switch (uHash)
	{
	case FNV1A::Hash32Const("atl"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_ATL;
	case FNV1A::Hash32Const("ord"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_ORD;
	case FNV1A::Hash32Const("dfw"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_DFW;
	case FNV1A::Hash32Const("lax"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_LAX;
	case FNV1A::Hash32Const("eat"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_EAT;
	case FNV1A::Hash32Const("jfk"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_JFK;
	case FNV1A::Hash32Const("sea"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_SEA;
	case FNV1A::Hash32Const("iad"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_IAD;
	case FNV1A::Hash32Const("ams"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_AMS;
	case FNV1A::Hash32Const("fra"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_FRA;
	case FNV1A::Hash32Const("hel"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_HEL;
	case FNV1A::Hash32Const("lhr"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_LHR;
	case FNV1A::Hash32Const("mad"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_MAD;
	case FNV1A::Hash32Const("par"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_PAR;
	case FNV1A::Hash32Const("sto"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_STO;
	case FNV1A::Hash32Const("sto2"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_STO;
	case FNV1A::Hash32Const("vie"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_VIE;
	case FNV1A::Hash32Const("waw"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_WAW;
	case FNV1A::Hash32Const("eze"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_EZE;
	case FNV1A::Hash32Const("lim"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_LIM;
	case FNV1A::Hash32Const("scl"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_SCL;
	case FNV1A::Hash32Const("gru"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_GRU;
	case FNV1A::Hash32Const("bom2"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_BOM2;
	case FNV1A::Hash32Const("maa"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_MAA;
	case FNV1A::Hash32Const("dxb"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_DXB;
	case FNV1A::Hash32Const("hkg"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_HKG;
	case FNV1A::Hash32Const("maa2"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_MAA2;
	case FNV1A::Hash32Const("bom"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_BOM;
	case FNV1A::Hash32Const("seo"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_SEO;
	case FNV1A::Hash32Const("sgp"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_SGP;
	case FNV1A::Hash32Const("tyo"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_TYO;
	case FNV1A::Hash32Const("syd"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_SYD;
	case FNV1A::Hash32Const("jnb"): return Vars::Misc::Queueing::ForceRegionsEnum::DC_JNB;
	}
	return 0;
}

MAKE_HOOK(ISteamNetworkingUtils_GetPingToDataCenter, U::Memory.GetVFunc(I::SteamNetworkingUtils, 8), int,
	void* rcx, SteamNetworkingPOPID popID, SteamNetworkingPOPID* pViaRelayPoP)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::ISteamNetworkingUtils_GetPingToDataCenter[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, popID, pViaRelayPoP);
#endif

	int iReturn = CALL_ORIGINAL(rcx, popID, pViaRelayPoP);
	if (!Vars::Misc::Queueing::ForceRegions.Value || iReturn < 0)
		return iReturn;

	char popIDName[5];
	POPID_ToString(popID, popIDName);
	if (auto uDatacenter = GetDatacenter(FNV1A::Hash32(popIDName)))
		return Vars::Misc::Queueing::ForceRegions.Value & uDatacenter ? 1 : 1000;

	return iReturn;
}

MAKE_HOOK(CTFPartyClient_RequestQueueForMatch, S::CTFPartyClient_RequestQueueForMatch(), void,
	void* rcx, int eMatchGroup)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::ISteamNetworkingUtils_GetPingToDataCenter[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, eMatchGroup);
#endif

	I::TFGCClientSystem->SetPendingPingRefresh(true);
	I::TFGCClientSystem->PingThink();

	CALL_ORIGINAL(rcx, eMatchGroup);
}