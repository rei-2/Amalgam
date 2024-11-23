#include "../SDK/SDK.h"

MAKE_SIGNATURE(CClientState_GetTime, "engine.dll", "80 B9 ? ? ? ? ? 66 0F 6E 81", 0x0);
MAKE_SIGNATURE(CL_FireEvents_GetTime_Call, "engine.dll", "0F 2F 45 ? 0F 82", 0x0);

MAKE_HOOK(CClientState_GetTime, S::CClientState_GetTime(), float,
	void* rcx)
{
	static const auto dwDesired = S::CL_FireEvents_GetTime_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	if (dwRetAddr == dwDesired)
		return std::numeric_limits<float>::max();

	return CALL_ORIGINAL(rcx);
}