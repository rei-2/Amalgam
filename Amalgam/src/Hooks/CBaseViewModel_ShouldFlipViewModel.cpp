#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseViewModel_ShouldFlipViewModel, "client.dll", "8B 91 ? ? ? ? 85 D2 74 ? B8 ? ? ? ? 83 FA ? 74 ? 0F B7 C2 4C 8B 05 ? ? ? ? 8B C8 49 83 C0 ? 48 C1 E1 ? 4C 03 C1 74 ? C1 EA ? 41 39 50 ? 75 ? 49 8B 08 48 85 C9 74 ? 48 8B 05", 0x0);

MAKE_HOOK(CBaseViewModel_ShouldFlipViewModel, S::CBaseViewModel_ShouldFlipViewModel(), bool,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBaseViewModel_ShouldFlipViewModel[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	return G::FlipViewmodels = CALL_ORIGINAL(rcx);
}