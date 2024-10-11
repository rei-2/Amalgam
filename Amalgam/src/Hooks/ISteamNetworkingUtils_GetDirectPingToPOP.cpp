#include "../SDK/SDK.h"

void POPID_ToString(SteamNetworkingPOPID popID, char* out)
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
	case FNV1A::Hash32Const("atl"): return DC_ATL;
	case FNV1A::Hash32Const("ord"): return DC_ORD;
	case FNV1A::Hash32Const("dfw"): return DC_DFW;
	case FNV1A::Hash32Const("lax"): return DC_LAX;
	case FNV1A::Hash32Const("eat"): return DC_EAT;
	case FNV1A::Hash32Const("jfk"): return DC_JFK;
	case FNV1A::Hash32Const("sea"): return DC_SEA;
	case FNV1A::Hash32Const("iad"): return DC_IAD;
	case FNV1A::Hash32Const("ams"): return DC_AMS;
	case FNV1A::Hash32Const("fra"): return DC_FRA;
	case FNV1A::Hash32Const("hel"): return DC_HEL;
	case FNV1A::Hash32Const("lhr"): return DC_LHR;
	case FNV1A::Hash32Const("mad"): return DC_MAD;
	case FNV1A::Hash32Const("par"): return DC_PAR;
	case FNV1A::Hash32Const("sto"): return DC_STO;
	case FNV1A::Hash32Const("sto2"): return DC_STO;
	case FNV1A::Hash32Const("vie"): return DC_VIE;
	case FNV1A::Hash32Const("waw"): return DC_WAW;
	case FNV1A::Hash32Const("eze"): return DC_EZE;
	case FNV1A::Hash32Const("lim"): return DC_LIM;
	case FNV1A::Hash32Const("scl"): return DC_SCL;
	case FNV1A::Hash32Const("gru"): return DC_GRU;
	case FNV1A::Hash32Const("bom2"): return DC_BOM2;
	case FNV1A::Hash32Const("maa"): return DC_MAA;
	case FNV1A::Hash32Const("dxb"): return DC_DXB;
	case FNV1A::Hash32Const("hkg"): return DC_HKG;
	case FNV1A::Hash32Const("maa2"): return DC_MAA2;
	case FNV1A::Hash32Const("bom"): return DC_BOM;
	case FNV1A::Hash32Const("seo"): return DC_SEO;
	case FNV1A::Hash32Const("sgp"): return DC_SGP;
	case FNV1A::Hash32Const("tyo"): return DC_TYO;
	case FNV1A::Hash32Const("syd"): return DC_SYD;
	case FNV1A::Hash32Const("jnb"): return DC_JNB;
	}
	return 0;
}

MAKE_HOOK(ISteamNetworkingUtils_GetDirectPingToPOP, U::Memory.GetVFunc(I::SteamNetworkingUtils, 9), int, __fastcall,
	void* rcx, SteamNetworkingPOPID popID)
{
	int iOriginal = CALL_ORIGINAL(rcx, popID);
	if (!Vars::Misc::Queueing::ForceRegions.Value)
		return iOriginal;

	char popIDName[5];
	POPID_ToString(popID, popIDName);
	if (auto uDatacenter = GetDatacenter(FNV1A::Hash32(popIDName)))
		return Vars::Misc::Queueing::ForceRegions.Value & uDatacenter ? 1 : 999999;

	return iOriginal;
}