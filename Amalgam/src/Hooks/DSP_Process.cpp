#include "../SDK/SDK.h"

MAKE_SIGNATURE(DSP_Process, "engine.dll", "48 89 5C 24 ? 55 41 54 41 57 48 83 EC ? 48 63 D9", 0x0);

MAKE_HOOK(DSP_Process, S::DSP_Process(), void,
	unsigned int idsp, int* pbfront, int* pbrear, int* pbcenter, int sampleCount)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::DSP_Process[DEFAULT_BIND])
		return CALL_ORIGINAL(idsp, pbfront, pbrear, pbcenter, sampleCount);
#endif

	if (Vars::Misc::Sound::RemoveDSP.Value)
	{
		// Skip DSP processing but still allow audio to pass through
		// by clearing the DSP ID (set to 0 for no processing)
		CALL_ORIGINAL(0, pbfront, pbrear, pbcenter, sampleCount);
	}
	else
	{
		// Normal DSP processing
		CALL_ORIGINAL(idsp, pbfront, pbrear, pbcenter, sampleCount);
	}
}