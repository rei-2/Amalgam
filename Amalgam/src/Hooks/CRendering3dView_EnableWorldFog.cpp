#include "../SDK/SDK.h"

MAKE_SIGNATURE(CRendering3dView_EnableWorldFog, "client.dll", "40 53 48 83 EC ? 48 8B 0D ? ? ? ? 48 89 7C 24", 0x0);

MAKE_HOOK(CRendering3dView_EnableWorldFog, S::CRendering3dView_EnableWorldFog(), void, __cdecl,
	)
{
	if (!(Vars::Visuals::World::Modulations.Value & 1 << 4) || I::EngineClient->IsTakingScreenshot() && Vars::Visuals::UI::CleanScreenshots.Value)
		return CALL_ORIGINAL();

	if (!Vars::Colors::FogModulation.Value.a)
		return;

	CALL_ORIGINAL();
	if (const auto pRenderContext = I::MaterialSystem->GetRenderContext())
	{
		float blend[3] = { float(Vars::Colors::FogModulation.Value.r) / 255.f, float(Vars::Colors::FogModulation.Value.g) / 255.f, float(Vars::Colors::FogModulation.Value.b) / 255.f };
		pRenderContext->FogColor3fv(blend);
	}
}