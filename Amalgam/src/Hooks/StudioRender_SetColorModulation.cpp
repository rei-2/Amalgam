#include "../SDK/SDK.h"

MAKE_HOOK(StudioRender_SetColorModulation, U::Memory.GetVFunc(I::StudioRender, 27), void, __fastcall,
	void* ecx, const float* pColor)
{
	if (Vars::Visuals::World::Modulations.Value & (1 << 2) && G::DrawingProps && !(Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot()))
	{
		const float flCustomBlend[3] = {
			float(Vars::Colors::PropModulation.Value.r) / 255.f,
			float(Vars::Colors::PropModulation.Value.g) / 255.f,
			float(Vars::Colors::PropModulation.Value.b) / 255.f
		};

		return CALL_ORIGINAL(ecx, flCustomBlend);
	}

	CALL_ORIGINAL(ecx, pColor);
}