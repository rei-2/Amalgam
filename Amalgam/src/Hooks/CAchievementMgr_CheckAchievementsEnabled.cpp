#include "../SDK/SDK.h"

MAKE_SIGNATURE(CAchievementMgr_CheckAchievementsEnabled, "client.dll", "40 53 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B D9 48 8B 48 ? 48 85 C9 0F 84", 0x0);

MAKE_HOOK(CAchievementMgr_CheckAchievementsEnabled, S::CAchievementMgr_CheckAchievementsEnabled(), bool, __fastcall,
	void* ecx)
{
	return true;
}