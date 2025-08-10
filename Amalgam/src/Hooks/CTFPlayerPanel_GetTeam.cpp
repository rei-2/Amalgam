#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CTFPlayerPanel_GetTeam, "client.dll", "8B 91 ? ? ? ? 83 FA ? 74 ? 48 8B 05", 0x0);
MAKE_SIGNATURE(CTFTeamStatusPlayerPanel_Update, "client.dll", "40 56 57 48 83 EC ? 48 83 3D", 0x0);
MAKE_SIGNATURE(vgui_Panel_SetBgColor, "client.dll", "89 91 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC 48 8B 41", 0x0);
MAKE_SIGNATURE(CTFTeamStatusPlayerPanel_Update_GetTeam_Call, "client.dll", "8B 9F ? ? ? ? 40 32 F6", 0x0);
MAKE_SIGNATURE(CTFTeamStatusPlayerPanel_Update_SetBgColor_Call, "client.dll", "48 8B 8F ? ? ? ? 4C 8B 6C 24 ? 48 85 C9 0F 84 ? ? ? ? 40 38 B7", 0x0);

static int s_iPlayerIndex;

static inline Color_t GetScoreboardColor(int iIndex)
{
	Color_t out = { 0, 0, 0, 0 };

	if (iIndex == I::EngineClient->GetLocalPlayer())
		out = Vars::Colors::Local.Value;
	else if (H::Entities.IsFriend(iIndex))
		out = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(FRIEND_TAG)].m_tColor;
	else if (H::Entities.InParty(iIndex))
		out = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(PARTY_TAG)].m_tColor;
	else if (auto pTag = F::PlayerUtils.GetSignificantTag(iIndex))
		out = pTag->m_tColor;

	return out;
}

MAKE_HOOK(CTFPlayerPanel_GetTeam, S::CTFPlayerPanel_GetTeam(), int,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayerPanel_GetTeam[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	static const auto dwDesired = S::CTFTeamStatusPlayerPanel_Update_GetTeam_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (Vars::Visuals::UI::RevealScoreboard.Value && dwRetAddr == dwDesired)
	{
		if (auto pLocal = H::Entities.GetLocal())
			return pLocal->m_iTeamNum();
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(CTFTeamStatusPlayerPanel_Update, S::CTFTeamStatusPlayerPanel_Update(), bool,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayerPanel_GetTeam[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	s_iPlayerIndex = *reinterpret_cast<int*>(uintptr_t(rcx) + 580);
	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(vgui_Panel_SetBgColor, S::vgui_Panel_SetBgColor(), void,
	void* rcx, Color_t color)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayerPanel_GetTeam[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, color);
#endif

	static const auto dwDesired = S::CTFTeamStatusPlayerPanel_Update_SetBgColor_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (dwRetAddr == dwDesired && Vars::Visuals::UI::ScoreboardColors.Value)
	{
		Color_t tColor = GetScoreboardColor(s_iPlayerIndex);
		if (tColor.a)
		{
			auto pResource = H::Entities.GetResource();
			if (pResource && !pResource->m_bAlive(s_iPlayerIndex))
				tColor = tColor.Lerp({ 127, 127, 127, tColor.a }, 0.5f);

			color = tColor;
		}
	}

	CALL_ORIGINAL(rcx, color);
}