#include "../SDK/SDK.h"

MAKE_SIGNATURE(CViewRender_PerformScreenOverlay, "client.dll", "4C 8B DC 49 89 5B ? 89 54 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 81 EC", 0x0);

MAKE_HOOK(CViewRender_PerformScreenOverlay, S::CViewRender_PerformScreenOverlay(), void,
	void* rcx, int x, int y, int w, int h)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CViewRender_PerformScreenOverlay[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, x, y, w, h);
#endif

	if (!Vars::Visuals::Removals::ScreenOverlays.Value || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		CALL_ORIGINAL(rcx, x, y, w, h);
}