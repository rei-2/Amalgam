#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseClient_SetUpdateRate, "engine.dll", "83 FA ? 7D ? BA ? ? ? ? EB ? B8", 0x0);

MAKE_HOOK(CBaseClient_SetUpdateRate, S::CBaseClient_SetUpdateRate(), void, __fastcall,
	void* rcx, int updaterate, bool bForce)
{
	return CALL_ORIGINAL(rcx, 67, bForce); // seems localhost only
}