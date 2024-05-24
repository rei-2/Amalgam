#include "../SDK/SDK.h"

MAKE_SIGNATURE(DoEnginePostProcessing, "client.dll", "48 8B C4 44 89 48 ? 44 89 40 ? 89 50 ? 89 48", 0x0);

MAKE_HOOK(DoEnginePostProcessing, S::DoEnginePostProcessing(), void, __cdecl,
	int x, int y, int w, int h, bool bFlashlightIsOn, bool bPostVGui)
{
	if (!Vars::Visuals::Removals::PostProcessing.Value || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		CALL_ORIGINAL(x, y, w, h, bFlashlightIsOn, bPostVGui);
}