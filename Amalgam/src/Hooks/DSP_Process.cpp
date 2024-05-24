#include "../SDK/SDK.h"

MAKE_SIGNATURE(DSP_Process, "engine.dll", "48 89 5C 24 ? 55 41 54 41 57 48 83 EC ? 48 63 D9", 0x0);

MAKE_HOOK(DSP_Process, S::DSP_Process(), void, __cdecl,
	unsigned int idsp, int* pbfront, int* pbrear, int* pbcenter, int sampleCount)
{
	if (!Vars::Visuals::Removals::DSP.Value)
		CALL_ORIGINAL(idsp, pbfront, pbrear, pbcenter, sampleCount);
}