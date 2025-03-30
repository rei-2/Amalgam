#include "../SDK/SDK.h"

#include "../Features/NetworkFix/NetworkFix.h"

MAKE_SIGNATURE(CL_ReadPackets, "engine.dll", "4C 8B DC 49 89 5B ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B 05", 0x0);

MAKE_HOOK(CL_ReadPackets, S::CL_ReadPackets(), void,
	bool bFinalTick)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CL_ReadPackets[DEFAULT_BIND])
		return CALL_ORIGINAL(bFinalTick);
#endif

	if (F::NetworkFix.ShouldReadPackets())
		CALL_ORIGINAL(bFinalTick);
}