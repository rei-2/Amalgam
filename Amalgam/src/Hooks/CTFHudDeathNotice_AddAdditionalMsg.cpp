#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFHudDeathNotice_AddAdditionalMsg, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 49 8B D9 41 8B E8", 0x0);
MAKE_SIGNATURE(CLocalizedStringTable_Find, "vgui2.dll", "40 53 48 83 EC ? 48 8B 01 48 8B D9 FF 50 ? 83 F8", 0x0);

static bool bShouldOverride = false;
static std::wstring sOverride;

MAKE_HOOK(CTFHudDeathNotice_AddAdditionalMsg, S::CTFHudDeathNotice_AddAdditionalMsg(), void,
	void* rcx, int iKillerID, int iVictimID, const char* pMsgKey)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFHudDeathNotice_AddAdditionalMsg.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, iKillerID, iVictimID, pMsgKey);
#endif

	if (iKillerID == I::EngineClient->GetLocalPlayer() && (!Vars::Visuals::Misc::LocalDominationOverride.Value.empty() || !Vars::Visuals::Misc::LocalRevengeOverride.Value.empty())
		|| (!Vars::Visuals::Misc::DominationOverride.Value.empty() || !Vars::Visuals::Misc::RevengeOverride.Value.empty()))
	{
		switch (FNV1A::Hash32(pMsgKey))
		{
		case FNV1A::Hash32Const("#Msg_Dominating"):
		case FNV1A::Hash32Const("#Msg_Dominating_What"):
			if (iKillerID == I::EngineClient->GetLocalPlayer() && !Vars::Visuals::Misc::LocalDominationOverride.Value.empty())
			{
				bShouldOverride = true;
				sOverride = SDK::ConvertUtf8ToWide(Vars::Visuals::Misc::LocalDominationOverride.Value);
			}
			else if (!Vars::Visuals::Misc::DominationOverride.Value.empty())
			{
				bShouldOverride = true;
				sOverride = SDK::ConvertUtf8ToWide(Vars::Visuals::Misc::DominationOverride.Value);
			}
			break;
		case FNV1A::Hash32Const("#Msg_Revenge"):
		case FNV1A::Hash32Const("#Msg_Revenge_What"):
			if (iKillerID == I::EngineClient->GetLocalPlayer() && !Vars::Visuals::Misc::LocalRevengeOverride.Value.empty())
			{
				bShouldOverride = true;
				sOverride = SDK::ConvertUtf8ToWide(Vars::Visuals::Misc::LocalRevengeOverride.Value);
			}
			else if (!Vars::Visuals::Misc::RevengeOverride.Value.empty())
			{
				bShouldOverride = true;
				sOverride = SDK::ConvertUtf8ToWide(Vars::Visuals::Misc::RevengeOverride.Value);
			}
		}
		if (bShouldOverride)
		{
			CALL_ORIGINAL(rcx, iKillerID, iVictimID, pMsgKey);
			bShouldOverride = false;
			return;
		}
	}
	CALL_ORIGINAL(rcx, iKillerID, iVictimID, pMsgKey);
}

MAKE_HOOK(CLocalizedStringTable_Find, S::CLocalizedStringTable_Find(), wchar_t*,
	void* rcx, const char* pName)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFHudDeathNotice_AddAdditionalMsg.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pName);
#endif

	if (bShouldOverride)
		return (wchar_t*)sOverride.c_str();
	return CALL_ORIGINAL(rcx, pName);
}