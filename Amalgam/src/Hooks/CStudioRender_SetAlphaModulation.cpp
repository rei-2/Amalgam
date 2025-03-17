#include "../SDK/SDK.h"

MAKE_HOOK(CStudioRender_SetAlphaModulation, U::Memory.GetVFunc(I::StudioRender, 28), void,
	void* rcx, float flAlpha)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CStudioRender_SetAlphaModulation.Map[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, flAlpha);
#endif

	if (Vars::Visuals::World::Modulations.Value & Vars::Visuals::World::ModulationsEnum::Prop && G::DrawingProps && !(Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot()))
		return CALL_ORIGINAL(rcx, float(Vars::Colors::PropModulation.Value.a) / 255.f * flAlpha);

	CALL_ORIGINAL(rcx, flAlpha);
}