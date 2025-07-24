#include "../SDK/SDK.h"

#include "../Features/Spectate/Spectate.h"

MAKE_HOOK(IBaseClientDLL_LevelShutdown, U::Memory.GetVirtual(I::BaseClientDLL, 7), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::IBaseClientDLL_LevelShutdown[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	H::Entities.Clear(true);
	F::Spectate.m_iIntendedTarget = -1;

	CALL_ORIGINAL(rcx);
}