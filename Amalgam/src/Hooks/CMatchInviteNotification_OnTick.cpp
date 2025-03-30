#include "../SDK/SDK.h"

MAKE_SIGNATURE(CMatchInviteNotification_OnTick, "client.dll", "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? F7 83", 0x0);

MAKE_HOOK(CMatchInviteNotification_OnTick, S::CMatchInviteNotification_OnTick(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CMatchInviteNotification_OnTick[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	if (Vars::Misc::Queueing::FreezeQueue.Value)
		*reinterpret_cast<double*>(uintptr_t(rcx) + 616) = 0.0;

	CALL_ORIGINAL(rcx);
}