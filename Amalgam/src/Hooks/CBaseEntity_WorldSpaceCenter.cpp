#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseEntity_WorldSpaceCenter, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99 ? ? ? ? E8", 0x0);
MAKE_SIGNATURE(CDamageAccountPanel_DisplayDamageFeedback_CBaseEntity_WorldSpaceCenter_Call, "client.dll", "F3 0F 10 0B F3 0F 10 53 ? F3 0F 10 43 ? F3 0F 10 18", 0x0);
MAKE_SIGNATURE(MainViewOrigin, "client.dll", "48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 8D 05 ? ? ? ? C3 CC CC CC CC CC CC CC CC 48 83 EC ? 0F 29 34 24", 0x0);

MAKE_HOOK(CBaseEntity_WorldSpaceCenter, S::CBaseEntity_WorldSpaceCenter(), const Vec3&,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBaseEntity_WorldSpaceCenter[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	static const auto dwDesired = S::CDamageAccountPanel_DisplayDamageFeedback_CBaseEntity_WorldSpaceCenter_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	return dwRetAddr == dwDesired && Vars::Visuals::Effects::DrawDamageNumbersThroughWalls.Value ? S::MainViewOrigin.Call<const Vec3&>() : CALL_ORIGINAL(rcx);
}