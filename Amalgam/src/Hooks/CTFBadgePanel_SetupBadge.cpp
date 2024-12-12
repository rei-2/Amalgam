#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFBadgePanel_SetupBadge, "client.dll", "48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 48 89 7C 24 ? 4C 89 74 24", 0x0);

class IProgressionDesc;
struct LevelInfo_t
{
	uint32 m_nLevelNum;
	uint32 m_nStartXP;
	uint32 m_nEndXP;
	const char* m_pszLevelIcon;
	const char* m_pszLevelTitle;
	const char* m_pszLevelUpSound;
	const char* m_pszLobbyBackgroundImage;
};

MAKE_HOOK(CTFBadgePanel_SetupBadge, S::CTFBadgePanel_SetupBadge(), void,
	void* rdx, const IProgressionDesc* pProgress, /*const*/ LevelInfo_t& levelInfo, const CSteamID& steamID)
{
	//SDK::Output("SetupBadge", std::format("{}: {}", steamID.GetAccountID(), levelInfo.m_nLevelNum).c_str());

	if (!Vars::Visuals::UI::StreamerMode.Value)
		return CALL_ORIGINAL(rdx, pProgress, levelInfo, steamID);

	PlayerInfo_t pi{};
	if (!I::EngineClient->GetPlayerInfo(I::EngineClient->GetLocalPlayer(), &pi))
		return CALL_ORIGINAL(rdx, pProgress, levelInfo, steamID);

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
		return CALL_ORIGINAL(rdx, pProgress, levelInfo, steamID);

	int nOldLevelNum = levelInfo.m_nLevelNum;
	levelInfo.m_nLevelNum = 1;
	CALL_ORIGINAL(rdx, pProgress, levelInfo, steamID);
	levelInfo.m_nLevelNum = nOldLevelNum;
}