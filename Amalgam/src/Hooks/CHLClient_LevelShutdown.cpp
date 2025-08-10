#include "../SDK/SDK.h"

#include "../Features/Spectate/Spectate.h"

MAKE_HOOK(CHLClient_LevelShutdown, U::Memory.GetVirtual(I::Client, 7), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CHLClient_LevelShutdown[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	H::Entities.Clear(true);
	F::Spectate.m_iIntendedTarget = -1;

	CALL_ORIGINAL(rcx);
}