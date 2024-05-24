#include "../SDK/SDK.h"

/*
const std::unordered_map<std::string, unsigned> DATA_CENTER_HASH
{
	{ "atl", DC_ATL },
	{ "ord", DC_ORD },
	{ "dfw", DC_DFW },
	{ "lax", DC_LAX },
	{ "eat", DC_EAT },
	{ "jfk", DC_JFK },
	{ "sea", DC_SEA },
	{ "iad", DC_IAD },
	{ "ams", DC_AMS },
	{ "fra", DC_FRA },
	{ "hel", DC_HEL },
	{ "lhr", DC_LHR },
	{ "mad", DC_MAD },
	{ "par", DC_PAR },
	{ "sto", DC_STO },
	{ "sto2", DC_STO },
	{ "vie", DC_VIE },
	{ "waw", DC_WAW },
	{ "eze", DC_EZE },
	{ "lim", DC_LIM },
	{ "scl", DC_SCL },
	{ "gru", DC_GRU },
	{ "bom2", DC_BOM2 },
	{ "maa", DC_MAA },
	{ "dxb", DC_DXB },
	{ "hkg", DC_HKG },
	{ "maa2", DC_MAA2 },
	{ "bom", DC_BOM },
	{ "seo", DC_SEO },
	{ "sgp", DC_SGP },
	{ "tyo", DC_TYO },
	{ "syd", DC_SYD },
	{ "jnb", DC_JNB }
};
*/

void POPID_ToString(SteamNetworkingPOPID popID, char* out)
{
	out[0] = static_cast<char>(popID >> 16);
	out[1] = static_cast<char>(popID >> 8);
	out[2] = static_cast<char>(popID);
	out[3] = static_cast<char>(popID >> 24);
	out[4] = 0;
}

unsigned int GetDatacenter(FNV1A_t uHash)
{
	switch (uHash)
	{
	case FNV1A::HashConst("atl"): return DC_ATL;
	case FNV1A::HashConst("ord"): return DC_ORD;
	case FNV1A::HashConst("dfw"): return DC_DFW;
	case FNV1A::HashConst("lax"): return DC_LAX;
	case FNV1A::HashConst("eat"): return DC_EAT;
	case FNV1A::HashConst("jfk"): return DC_JFK;
	case FNV1A::HashConst("sea"): return DC_SEA;
	case FNV1A::HashConst("iad"): return DC_IAD;
	case FNV1A::HashConst("ams"): return DC_AMS;
	case FNV1A::HashConst("fra"): return DC_FRA;
	case FNV1A::HashConst("hel"): return DC_HEL;
	case FNV1A::HashConst("lhr"): return DC_LHR;
	case FNV1A::HashConst("mad"): return DC_MAD;
	case FNV1A::HashConst("par"): return DC_PAR;
	case FNV1A::HashConst("sto"): return DC_STO;
	case FNV1A::HashConst("sto2"): return DC_STO;
	case FNV1A::HashConst("vie"): return DC_VIE;
	case FNV1A::HashConst("waw"): return DC_WAW;
	case FNV1A::HashConst("eze"): return DC_EZE;
	case FNV1A::HashConst("lim"): return DC_LIM;
	case FNV1A::HashConst("scl"): return DC_SCL;
	case FNV1A::HashConst("gru"): return DC_GRU;
	case FNV1A::HashConst("bom2"): return DC_BOM2;
	case FNV1A::HashConst("maa"): return DC_MAA;
	case FNV1A::HashConst("dxb"): return DC_DXB;
	case FNV1A::HashConst("hkg"): return DC_HKG;
	case FNV1A::HashConst("maa2"): return DC_MAA2;
	case FNV1A::HashConst("bom"): return DC_BOM;
	case FNV1A::HashConst("seo"): return DC_SEO;
	case FNV1A::HashConst("sgp"): return DC_SGP;
	case FNV1A::HashConst("tyo"): return DC_TYO;
	case FNV1A::HashConst("syd"): return DC_SYD;
	case FNV1A::HashConst("jnb"): return DC_JNB;
	}
	return 0;
}

MAKE_HOOK(ISteamNetworkingUtils_GetDirectPingToPOP, U::Memory.GetVFunc(I::SteamNetworkingUtils, 9), int, __fastcall,
	void* ecx, SteamNetworkingPOPID popID)
{
	int iOriginal = CALL_ORIGINAL(ecx, popID);
	if (!Vars::Misc::Queueing::ForceRegions.Value)
		return iOriginal;

	char popIDName[5];
	POPID_ToString(popID, popIDName);
	if (auto uDatacenter = GetDatacenter(FNV1A::Hash(popIDName)))
		return Vars::Misc::Queueing::ForceRegions.Value & uDatacenter ? 1 : 999999;

	return iOriginal;
}