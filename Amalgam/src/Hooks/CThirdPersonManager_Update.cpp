#include "../SDK/SDK.h"

MAKE_SIGNATURE(CThirdPersonManager_Update, "client.dll", "40 53 48 83 EC ? 48 8B 05 ? ? ? ? 48 8B D9 48 85 C0 75 ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 01 FF 50 ? 48 89 05 ? ? ? ? 48 85 C0 74 ? 48 8B 40 ? 83 78", 0x0);

MAKE_HOOK(CThirdPersonManager_Update, S::CThirdPersonManager_Update(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CThirdPersonManager_Update[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif
}