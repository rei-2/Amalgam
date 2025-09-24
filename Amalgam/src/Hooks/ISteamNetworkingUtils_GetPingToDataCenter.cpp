#include "../SDK/SDK.h"

// we could maybe use tf_datacenter_ping_interval?

static void PopIdName(SteamNetworkingPOPID popID, char* out)
{
	out[0] = static_cast<char>(popID >> 16);
	out[1] = static_cast<char>(popID >> 8);
	out[2] = static_cast<char>(popID);
	out[3] = static_cast<char>(popID >> 24);
	out[4] = 0;
}

static inline int GetDatacenter(uint32_t uHash)
{
	switch (uHash)
	{
	case FNV1A::Hash32Const("atl"): return Vars::Misc::Queueing::ForceRegionsEnum::ATL;
	case FNV1A::Hash32Const("ord"): return Vars::Misc::Queueing::ForceRegionsEnum::ORD;
	case FNV1A::Hash32Const("dfw"): return Vars::Misc::Queueing::ForceRegionsEnum::DFW;
	case FNV1A::Hash32Const("lax"): return Vars::Misc::Queueing::ForceRegionsEnum::LAX;
	case FNV1A::Hash32Const("sea"):
	case FNV1A::Hash32Const("eat"): return Vars::Misc::Queueing::ForceRegionsEnum::SEA;
	case FNV1A::Hash32Const("iad"): return Vars::Misc::Queueing::ForceRegionsEnum::IAD;
	case FNV1A::Hash32Const("ams"):
	case FNV1A::Hash32Const("ams4"): return Vars::Misc::Queueing::ForceRegionsEnum::AMS;
	case FNV1A::Hash32Const("fsn"): return Vars::Misc::Queueing::ForceRegionsEnum::FSN;
	case FNV1A::Hash32Const("fra"): return Vars::Misc::Queueing::ForceRegionsEnum::FRA;
	case FNV1A::Hash32Const("hel"): return Vars::Misc::Queueing::ForceRegionsEnum::HEL;
	case FNV1A::Hash32Const("lhr"): return Vars::Misc::Queueing::ForceRegionsEnum::LHR;
	case FNV1A::Hash32Const("mad"): return Vars::Misc::Queueing::ForceRegionsEnum::MAD;
	case FNV1A::Hash32Const("par"): return Vars::Misc::Queueing::ForceRegionsEnum::PAR;
	case FNV1A::Hash32Const("sto"):
	case FNV1A::Hash32Const("sto2"): return Vars::Misc::Queueing::ForceRegionsEnum::STO;
	case FNV1A::Hash32Const("vie"): return Vars::Misc::Queueing::ForceRegionsEnum::VIE;
	case FNV1A::Hash32Const("waw"): return Vars::Misc::Queueing::ForceRegionsEnum::WAW;
	case FNV1A::Hash32Const("eze"): return Vars::Misc::Queueing::ForceRegionsEnum::EZE;
	case FNV1A::Hash32Const("lim"): return Vars::Misc::Queueing::ForceRegionsEnum::LIM;
	case FNV1A::Hash32Const("scl"): return Vars::Misc::Queueing::ForceRegionsEnum::SCL;
	case FNV1A::Hash32Const("gru"): return Vars::Misc::Queueing::ForceRegionsEnum::GRU;
	case FNV1A::Hash32Const("maa2"): return Vars::Misc::Queueing::ForceRegionsEnum::MAA;
	case FNV1A::Hash32Const("dxb"): return Vars::Misc::Queueing::ForceRegionsEnum::DXB;
	case FNV1A::Hash32Const("hkg"): return Vars::Misc::Queueing::ForceRegionsEnum::HKG;
	case FNV1A::Hash32Const("bom2"): return Vars::Misc::Queueing::ForceRegionsEnum::BOM;
	case FNV1A::Hash32Const("seo"): return Vars::Misc::Queueing::ForceRegionsEnum::SEO;
	case FNV1A::Hash32Const("sgp"): return Vars::Misc::Queueing::ForceRegionsEnum::SGP;
	case FNV1A::Hash32Const("tyo"): return Vars::Misc::Queueing::ForceRegionsEnum::TYO;
	case FNV1A::Hash32Const("syd"): return Vars::Misc::Queueing::ForceRegionsEnum::SYD;
	case FNV1A::Hash32Const("jnb"): return Vars::Misc::Queueing::ForceRegionsEnum::JNB;
	}
	return 0;
}

MAKE_HOOK(ISteamNetworkingUtils_GetPingToDataCenter, U::Memory.GetVirtual(I::SteamNetworkingUtils, 8), int,
	void* rcx, SteamNetworkingPOPID popID, SteamNetworkingPOPID* pViaRelayPoP)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::ISteamNetworkingUtils_GetPingToDataCenter[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, popID, pViaRelayPoP);
#endif

	int iReturn = CALL_ORIGINAL(rcx, popID, pViaRelayPoP);
	if (!Vars::Misc::Queueing::ForceRegions.Value || iReturn < 0)
		return iReturn;

	char sPopID[5];
	PopIdName(popID, sPopID);
	if (auto uDatacenter = GetDatacenter(FNV1A::Hash32(sPopID)))
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