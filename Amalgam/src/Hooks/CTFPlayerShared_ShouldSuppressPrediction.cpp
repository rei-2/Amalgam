#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerShared_ShouldSuppressPrediction, "client.dll", "8B 81 ? ? ? ? 0F BA E0", 0x0);

MAKE_HOOK(CTFPlayerShared_ShouldSuppressPrediction, S::CTFPlayerShared_ShouldSuppressPrediction(), bool,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayerShared_ShouldSuppressPrediction[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	return false;
}