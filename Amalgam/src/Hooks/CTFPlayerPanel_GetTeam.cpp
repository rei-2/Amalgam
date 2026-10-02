#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CTFPlayerPanel_GetTeam, "client.dll", "8B 91 ? ? ? ? 83 FA ? 74 ? 48 8B 05", 0x0);
MAKE_SIGNATURE(vgui_Panel_SetBgColor, "client.dll", "89 91 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC 48 8B 41", 0x0);
MAKE_SIGNATURE(CTFTeamStatusPlayerPanel_Update_GetTeam_Call, "client.dll", "8B 9F ? ? ? ? 40 32 F6", 0x0);
MAKE_SIGNATURE(CTFTeamStatusPlayerPanel_Update_SetBgColor_Call, "client.dll", "48 8B 8F ? ? ? ? 4C 8B 6C 24 ? 48 85 C9 0F 84 ? ? ? ? 40 38 B7", 0x0);
MAKE_SIGNATURE(CTFTeamStatus_OnTick, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B 01 48 8B F9 FF 90 ? ? ? ? 48 8B CF 0F B6 D8", 0x0);
MAKE_SIGNATURE(CVGui_RunFrame, "vgui2.dll", "48 8B C4 53 48 81 EC", 0x0);

static int s_iPlayerIndex;
static void* s_pTeamStatus;

void ResetTeamStatusPointer()
{
	s_pTeamStatus = nullptr;
}

static inline void SetScoreboardColor(int iIndex, Color_t& tColor)
{
	if (iIndex == I::EngineClient->GetLocalPlayer())
		tColor = Vars::Colors::Local.Value;
	else if (H::Entities.IsFriend(iIndex))
		tColor = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(FRIEND_TAG)].m_tColor;
	else if (H::Entities.InParty(iIndex))
		tColor = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(PARTY_TAG)].m_tColor;
	else if (auto pTag = F::PlayerUtils.GetSignificantTag(iIndex))
		tColor = pTag->m_tColor;
	else
		return;

	auto pResource = H::Entities.GetResource();
	if (pResource && !pResource->m_bAlive(iIndex))
		tColor = tColor.Lerp({ 127, 127, 127, tColor.a }, 0.5f);
}

MAKE_HOOK(CTFPlayerPanel_GetTeam, S::CTFPlayerPanel_GetTeam(), int,
	void* rcx)
{
	DEBUG_RETURN(CTFPlayerPanel_GetTeam, rcx);

	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	const auto dwDesired = S::CTFTeamStatusPlayerPanel_Update_GetTeam_Call();

	int iReturn = CALL_ORIGINAL(rcx);

	if (auto pResource = H::Entities.GetResource(); dwRetAddr == dwDesired && pResource)
	{
		// the offsets below are tied to the exact layout of CTFPlayerPanel,
		// sanity check them so a game update cannot silently corrupt the panel
		const int iPlayerIndex = *reinterpret_cast<int*>(uintptr_t(rcx) + 580);
		if (iPlayerIndex > 0 && iPlayerIndex < MAX_PLAYERS)
		{
			s_iPlayerIndex = iPlayerIndex;

			int iLocalTeam = pResource->m_iTeam(I::EngineClient->GetLocalPlayer());

			if (Vars::Visuals::UI::RevealScoreboard.Value && !SDK::CleanScreenshot())
				iReturn = iLocalTeam;

			const auto pHealthBar = *reinterpret_cast<void**>(uintptr_t(rcx) + 688);
			const auto pHealthBarVTable = pHealthBar ? *reinterpret_cast<void**>(pHealthBar) : nullptr;
			if (pHealthBarVTable && U::Memory.GetOffsetFromBase(uintptr_t(pHealthBarVTable)) != uintptr_t(-1) // vtable has to belong to a module
				&& U::Memory.CallVirtual<34, bool>(pHealthBar) != (iReturn == iLocalTeam))
				*reinterpret_cast<int*>(uintptr_t(rcx) + 624) = -1;
		}
	}

	return iReturn;
}

MAKE_HOOK(vgui_Panel_SetBgColor, S::vgui_Panel_SetBgColor(), void,
	void* rcx, Color_t color)
{
	DEBUG_RETURN(vgui_Panel_SetBgColor, rcx, color);

	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	const auto dwDesired = S::CTFTeamStatusPlayerPanel_Update_SetBgColor_Call();

	if (dwRetAddr == dwDesired && Vars::Visuals::UI::ScoreboardColors.Value && !SDK::CleanScreenshot())
		SetScoreboardColor(s_iPlayerIndex, color);

	CALL_ORIGINAL(rcx, color);
}

MAKE_HOOK(CTFTeamStatus_OnTick, S::CTFTeamStatus_OnTick(), void,
	void* rcx)
{
	DEBUG_RETURN(CTFTeamStatus_OnTick, rcx);

	s_pTeamStatus = rcx;

	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(CVGui_RunFrame, S::CVGui_RunFrame(), void,
	void* rcx)
{
	DEBUG_RETURN(CVGui_RunFrame, rcx);

	if (!s_pTeamStatus || !I::EngineClient->IsInGame())
		return CALL_ORIGINAL(rcx);

	static bool bStaticMod = false;
	const bool bLastMod = bStaticMod;
	const bool bCurrMod = bStaticMod = (Vars::Visuals::UI::RevealScoreboard.Value || Vars::Visuals::UI::ScoreboardColors.Value) && !SDK::CleanScreenshot();

	if (bCurrMod != bLastMod)
		S::CTFTeamStatus_OnTick.Call<void>(s_pTeamStatus);

	CALL_ORIGINAL(rcx);
}