#include "../SDK/SDK.h"

MAKE_HOOK(CStudioRender_SetColorModulation, U::Memory.GetVFunc(I::StudioRender, 27), void, __fastcall,
	void* rcx, const float* pColor)
{
	if (Vars::Visuals::World::Modulations.Value & Vars::Visuals::World::ModulationsEnum::Prop && G::DrawingProps && !(Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot()))
	{
		const float flCustomBlend[3] = {
			float(Vars::Colors::PropModulation.Value.r) / 255.f,
			float(Vars::Colors::PropModulation.Value.g) / 255.f,
			float(Vars::Colors::PropModulation.Value.b) / 255.f
		};

		return CALL_ORIGINAL(rcx, flCustomBlend);
	}

	CALL_ORIGINAL(rcx, pColor);
}