#include "../SDK/SDK.h"

#include "../Features/EnginePrediction/EnginePrediction.h"
#include "../Features/Spectate/Spectate.h"

MAKE_HOOK(CHLClient_LevelShutdown, U::Memory.GetVirtual(I::Client, 7), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CHLClient_LevelShutdown[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	H::Entities.Clear(true);
	F::EnginePrediction.Unload();
	F::Spectate.m_iIntendedTarget = -1;

	CALL_ORIGINAL(rcx);
}