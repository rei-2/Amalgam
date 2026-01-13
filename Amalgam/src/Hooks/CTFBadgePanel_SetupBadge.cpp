#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CTFBadgePanel_SetupBadge, "client.dll", "48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 48 89 7C 24 ? 4C 89 74 24", 0x0);

MAKE_HOOK(CTFBadgePanel_SetupBadge, S::CTFBadgePanel_SetupBadge(), void,
	void* rcx, const IMatchGroupDescription* pMatchDesc, /*const*/ LevelInfo_t& levelInfo, const CSteamID& steamID)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFBadgePanel_SetupBadge[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pMatchDesc, levelInfo, steamID);
#endif

	int nOldLevelNum = levelInfo.m_nLevelNum;

	switch (F::PlayerUtils.GetNameType(steamID.GetAccountID()))
	{	// probably only need to worry about local, friends, a/o party
	case NameTypeEnum::Local:
	case NameTypeEnum::Friend:
	case NameTypeEnum::Party:
		levelInfo.m_nLevelNum = 1;
		CALL_ORIGINAL(rcx, pMatchDesc, levelInfo, steamID);
		break;
	default:
		if (int iLevel = H::Entities.GetLevel(steamID.GetAccountID()); iLevel > 0)
			levelInfo.m_nLevelNum = iLevel;
		CALL_ORIGINAL(rcx, pMatchDesc, levelInfo, steamID);
	}

	levelInfo.m_nLevelNum = nOldLevelNum;
}