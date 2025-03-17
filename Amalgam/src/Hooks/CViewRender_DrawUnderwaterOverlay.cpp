#include "../SDK/SDK.h"

MAKE_SIGNATURE(CViewRender_DrawUnderwaterOverlay, "client.dll", "4C 8B DC 41 56 48 81 EC ? ? ? ? 4C 8B B1", 0x0);

MAKE_HOOK(CViewRender_DrawUnderwaterOverlay, S::CViewRender_DrawUnderwaterOverlay(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CViewRender_DrawUnderwaterOverlay.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	if (!Vars::Visuals::Removals::ScreenOverlays.Value || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		CALL_ORIGINAL(rcx);
}