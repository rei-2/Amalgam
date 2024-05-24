#include "../SDK/SDK.h"

#include "../Features/Visuals/Visuals.h"

MAKE_SIGNATURE(S_StartDynamicSound, "engine.dll", "4C 8B DC 57 48 81 EC", 0x0);

MAKE_HOOK(S_StartDynamicSound, S::S_StartDynamicSound(), int, __cdecl,
	StartSoundParams_t& params)
{
	F::Visuals.ManualNetwork(params);
	return CALL_ORIGINAL(params);
}