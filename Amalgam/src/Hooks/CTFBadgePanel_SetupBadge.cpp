#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFBadgePanel_SetupBadge, "client.dll", "48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 48 89 7C 24 ? 4C 89 74 24", 0x0);
MAKE_SIGNATURE(CModelImagePanel_InvalidateImage, "client.dll", "40 53 48 83 EC ? 48 8B D9 48 8B 89 ? ? ? ? 48 85 C9 74 ? 48 8B 01 FF 50 ? 48 C7 83 ? ? ? ? ? ? ? ? 48 8B 8B", 0x0);
MAKE_SIGNATURE(vgui_Panel_GetBounds, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 1D ? ? ? ? 4D 8B F9", 0x0);

MAKE_HOOK(CTFBadgePanel_SetupBadge, S::CTFBadgePanel_SetupBadge(), void,
	void* rcx, const IMatchGroupDescription* pMatchDesc, /*const*/ LevelInfo_t& levelInfo, const CSteamID& steamID)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFBadgePanel_SetupBadge.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pMatchDesc, levelInfo, steamID);
#endif

	//SDK::Output("SetupBadge", std::format("{}: {}", steamID.GetAccountID(), levelInfo.m_nLevelNum).c_str(), {}, true, false, false, true);

	if (!Vars::Visuals::UI::StreamerMode.Value)
		return CALL_ORIGINAL(rcx, pMatchDesc, levelInfo, steamID);

	PlayerInfo_t pi{};
	if (!I::EngineClient->GetPlayerInfo(I::EngineClient->GetLocalPlayer(), &pi))
		return CALL_ORIGINAL(rcx, pMatchDesc, levelInfo, steamID);

	auto friendsID = steamID.GetAccountID();
	// probably only need to worry about local, friends, a/o party
	bool bShouldHide = false;
	if (pi.friendsID == friendsID)
		bShouldHide = Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Local;
	else if (H::Entities.IsFriend(friendsID))
		bShouldHide = Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Friends;
	else if (H::Entities.InParty(friendsID))
		bShouldHide = Vars::Visuals::UI::StreamerMode.Value >= Vars::Visuals::UI::StreamerModeEnum::Party;
	if (!bShouldHide)
		return CALL_ORIGINAL(rcx, pMatchDesc, levelInfo, steamID);

	int nOldLevelNum = levelInfo.m_nLevelNum;
	levelInfo.m_nLevelNum = 1;
	CALL_ORIGINAL(rcx, pMatchDesc, levelInfo, steamID);
	levelInfo.m_nLevelNum = nOldLevelNum;
}