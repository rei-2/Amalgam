#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerInventory_GetMaxItemCount, "client.dll", "40 53 48 83 EC ? 48 8B 89 ? ? ? ? BB", 0x0);

MAKE_HOOK(CTFPlayerInventory_GetMaxItemCount, S::CTFPlayerInventory_GetMaxItemCount(), int,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayerInventory_GetMaxItemCount[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	return !Vars::Misc::Exploits::BackpackExpander.Value ? CALL_ORIGINAL(rcx) : 4000;
}