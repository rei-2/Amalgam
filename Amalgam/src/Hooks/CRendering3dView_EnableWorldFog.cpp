#include "../SDK/SDK.h"

MAKE_SIGNATURE(CRendering3dView_EnableWorldFog, "client.dll", "40 53 48 83 EC ? 48 8B 0D ? ? ? ? 48 89 74 24", 0x0);

MAKE_HOOK(CRendering3dView_EnableWorldFog, S::CRendering3dView_EnableWorldFog(), void,
	)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CRendering3dView_EnableWorldFog[DEFAULT_BIND])
		return CALL_ORIGINAL();
#endif

	if (!(Vars::Visuals::World::Modulations.Value & Vars::Visuals::World::ModulationsEnum::Fog) || I::EngineClient->IsTakingScreenshot() && Vars::Visuals::UI::CleanScreenshots.Value)
		return CALL_ORIGINAL();

	CALL_ORIGINAL();
	if (auto pRenderContext = I::MaterialSystem->GetRenderContext())
	{
		if (Vars::Colors::FogModulation.Value.a)
		{
			pRenderContext->FogColor3ub(Vars::Colors::FogModulation.Value.r, Vars::Colors::FogModulation.Value.g, Vars::Colors::FogModulation.Value.b);

			float flRatio = 255.f / Vars::Colors::FogModulation.Value.a;
			float flStart, flEnd; pRenderContext->GetFogDistances(&flStart, &flEnd, nullptr);
			pRenderContext->FogStart(flStart * flRatio);
			pRenderContext->FogEnd(flEnd * flRatio);
		}
		else
			pRenderContext->FogMode(MATERIAL_FOG_NONE);
	}
}