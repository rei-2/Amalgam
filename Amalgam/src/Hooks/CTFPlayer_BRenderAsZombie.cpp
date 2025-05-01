#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayer_BRenderAsZombie, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 B9 ? ? ? ? E8 ? ? ? ? 84 C0", 0x0);
MAKE_SIGNATURE(CTFRagdoll_CreateTFRagdoll_BRenderAsZombie_Call, "client.dll", "E8 ? ? ? ? 84 C0 74 ? C6 87", 0x5);

MAKE_HOOK(CTFPlayer_BRenderAsZombie, S::CTFPlayer_BRenderAsZombie(), bool,
	void* rcx, bool bWeaponsCheck)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayer_BRenderAsZombie[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, bWeaponsCheck);
#endif

	static const auto dwDesired = S::CTFRagdoll_CreateTFRagdoll_BRenderAsZombie_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (Vars::Visuals::Removals::Gibs.Value && dwRetAddr == dwDesired)
		return true;

	return CALL_ORIGINAL(rcx, bWeaponsCheck);
}