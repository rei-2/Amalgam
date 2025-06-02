#include "../SDK/SDK.h"

#include "../Features/Misc/Misc.h"

MAKE_HOOK(IBaseClientDLL_LevelShutdown, U::Memory.GetVirtual(I::BaseClientDLL, 7), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IBaseClientDLL_LevelShutdown[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	H::Entities.Clear(true);

	CALL_ORIGINAL(rcx);
}