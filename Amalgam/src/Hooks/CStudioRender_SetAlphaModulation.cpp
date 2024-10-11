#include "../SDK/SDK.h"

MAKE_HOOK(CStudioRender_SetAlphaModulation, U::Memory.GetVFunc(I::StudioRender, 28), void, __fastcall,
	void* rcx, float flAlpha)
{
	if (Vars::Visuals::World::Modulations.Value & (1 << 2) && G::DrawingProps && !(Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot()))
		return CALL_ORIGINAL(rcx, float(Vars::Colors::PropModulation.Value.a) / 255.f * flAlpha);

	CALL_ORIGINAL(rcx, flAlpha);
}