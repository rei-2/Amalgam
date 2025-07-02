#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseEntity_ResetLatched, "client.dll", "40 56 48 83 EC ? 48 8B 01 48 8B F1 FF 90 ? ? ? ? 84 C0 75", 0x0);

MAKE_HOOK(CBaseEntity_ResetLatched, S::CBaseEntity_ResetLatched(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBaseEntity_ResetLatched[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	if (rcx == H::Entities.GetLocal())
		return;

	CALL_ORIGINAL(rcx);
}